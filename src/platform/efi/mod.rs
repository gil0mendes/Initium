// Keep this line to ensure the `mem*` functions are linked in.
extern crate rlibc;

extern crate uefi;

extern crate arch;
pub mod allocator;
extern crate spin;

use uefi::Status;
use uefi::{prelude::*, proto::loaded_image::LoadedImage};

use self::console::{ConsoleInDevice, ConsoleOutManager};
use self::memory::MemoryManager;
use self::video::EFIVideoManager;
use crate::loader_main;
use arch::ArchManager;
use common::command_manager;
use uefi::table::SystemTable;

pub mod console;
mod device;
mod disk;
mod memory;
mod video;

pub use console::{CONSOLE_IN, CONSOLE_OUT};
pub use video::VIDEO_MANAGER;

/// Reference to the system table.
///
/// This table is only fully safe to use until UEFI boot services have been exited. After that, some fields and methods
/// are unsafe to use, see the documentation of UEFI's ExitBootServices entry point for more details.
static mut SYSTEM_TABLE: Option<SystemTable<Boot>> = None;

/// Reference to the boot image handler.
///
/// This is used for some platform exit routines.
static mut IMAGE_HANDLE: Option<uefi::Handle> = None;

/// Reference for the loader image
///
/// This is used to determine which device where are booting from.
static mut LOADED_IMAGE: Option<&LoadedImage> = None;

/// Global logger object
static mut LOGGER: Option<uefi::logger::Logger> = None;

/// Get borrow reference for system table.
pub(crate) fn get_system_table() -> &'static SystemTable<Boot> {
    let option = unsafe { &SYSTEM_TABLE };
    option.as_ref().expect("System table not saved")
}

/// Get borrow reference for image handle.
pub(crate) fn get_image_handle() -> &'static uefi::Handle {
    let option = unsafe { &IMAGE_HANDLE };
    option.as_ref().expect("Image handle not saved")
}

/// Get borrow reference for loaded image.
pub(crate) fn get_loaded_image() -> &'static LoadedImage {
    let option = unsafe { &LOADED_IMAGE };
    option.as_ref().expect("Loaded image not saved")
}

/// Check if the UEFI where we are running on is compatible
/// with the loader.
fn check_revision(rev: uefi::table::Revision) {
    let (major, minor) = (rev.major(), rev.minor());

    info!("UEFI {}.{}", major, minor);

    assert!(major >= 2, "Running on an old, unsupported version of UEFI");
    assert!(
        minor >= 30,
        "Old version of UEFI 2, some features might not be available."
    );
}

unsafe fn init_logging(st: &SystemTable<Boot>) {
    let stdout = st.stdout();

    // Construct the logger.
    let logger = {
        LOGGER = Some(uefi::logger::Logger::new(stdout));
        LOGGER.as_ref().unwrap()
    };

    // Set the logger.
    log::set_logger(logger).unwrap(); // Can only fail if already initialized.

    // Log everything.
    log::set_max_level(log::LevelFilter::Info);
}

/// Entry point for EFI platforms
#[no_mangle]
pub extern "C" fn efi_main(image_handle: uefi::Handle, system_table: SystemTable<Boot>) -> Status {
    // Initialize arch code
    let mut arch_manager = ArchManager::new();
    arch_manager.init();

    unsafe {
        // setup system table singleton
        SYSTEM_TABLE = Some(system_table.unsafe_clone());

        // setup logging
        init_logging(&system_table);

        //setup memory allocator
        allocator::init(system_table.boot_services());
    }

    // Reset the console before continue
    system_table
        .stdout()
        .reset(false)
        .expect_success("Failed to reset stdout");

    check_revision(system_table.uefi_revision());

    // initialize command manager
    command_manager::init();

    // Get boot services
    let boot_services = system_table.boot_services();

    // Firmware is required to set a 5 minute watchdog timer before running an image. Disable it.
    boot_services
        .set_watchdog_timer(0, 0x10000, None)
        .expect("efi: could not disable watchdog timer")
        .unwrap();

    // Initialize memory manager
    let memory_manager = MemoryManager::new();
    memory_manager.init(&boot_services);

    // Get the loader image protocol
    let loader_image = boot_services
        .handle_protocol::<LoadedImage>(image_handle)
        .expect("efi: failed to retrieve `LoaderImage` protocol from handle")
        .unwrap();
    unsafe { LOADED_IMAGE = Some(&*loader_image.get()) };

    // save image handle
    unsafe { IMAGE_HANDLE = Some(image_handle) };

    // initialize the video system
    EFIVideoManager::init(boot_services);

    // Create a console manager
    ConsoleOutManager::init();
    ConsoleInDevice::init(boot_services);

    // TODO: remove this, is just for testing
    info!(
        "internal time: {}",
        arch_manager.time_manager.current_time()
    );

    // Call loader main function
    loader_main();

    Status::SUCCESS
}

/// Detect and register all the devices
pub fn target_device_probe() {
    disk::init();

    // TODO: start and register the network system
}

/// Reboot the system
pub fn target_reboot() -> ! {
    let system_table_option = unsafe { &SYSTEM_TABLE };
    let system_table = system_table_option.as_ref().unwrap();

    system_table.runtime_services().reset(
        uefi::table::runtime::ResetType::Warm,
        Status::SUCCESS,
        None,
    );
}
