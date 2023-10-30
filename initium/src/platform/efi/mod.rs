// Keep this line to ensure the `mem*` functions are linked in.
extern crate rlibc;

extern crate uefi;

extern crate arch;
pub mod allocator;
extern crate spin;

use core::ffi::c_void;
use core::ptr::{self, NonNull};
use core::sync::atomic::{AtomicPtr, Ordering};

use log::info;
use uefi::table::boot::{EventType, Tpl};
use uefi::{prelude::*, proto::loaded_image::LoadedImage};
use uefi::{Event, Status};

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
static SYSTEM_TABLE: AtomicPtr<c_void> = AtomicPtr::new(ptr::null_mut());

/// Reference to the boot image handler.
///
/// This is used for some platform exit routines.
static mut IMAGE_HANDLE: Option<NonNull<uefi::Handle>> = None;

/// Reference for the loader image
///
/// This is used to determine which device where are booting from.
static mut LOADED_IMAGE: Option<NonNull<LoadedImage>> = None;

/// Global logger object
static LOGGER: uefi::logger::Logger = uefi::logger::Logger::new();

#[must_use]
fn system_table_opt() -> Option<SystemTable<Boot>> {
    let ptr = SYSTEM_TABLE.load(Ordering::Acquire);

    // Safety: the `SYSTEM_TABLE` pointer either be null or a valid system table.
    //
    // Null is the initial value, as well as the value set when exiting boot services.
    unsafe { SystemTable::from_ptr(ptr) }
}

/// Get borrow reference for system table.
pub(crate) fn get_system_table() -> SystemTable<Boot> {
    system_table_opt().expect("The system table handle is not available")
}

/// Get borrow reference for image handle.
pub(crate) fn get_image_handle() -> NonNull<uefi::Handle> {
    unsafe { IMAGE_HANDLE.expect("Image handle not saved") }
}

/// Get borrow reference for loaded image.
pub(crate) fn get_loaded_image() -> NonNull<LoadedImage> {
    unsafe { LOADED_IMAGE.expect("Loaded image not saved") }
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

unsafe fn init_logging(st: &mut SystemTable<Boot>) {
    // Connect the logger to stdout
    LOGGER.set_output(st.stdout());

    // Set the logger.
    log::set_logger(&LOGGER).unwrap(); // Can only fail if already initialized.

    // Log everything.
    log::set_max_level(log::LevelFilter::Info);
}

#[entry]
fn efi_main(image_handle: uefi::Handle, mut system_table: SystemTable<Boot>) -> Status {
    // Initialize arch code
    let mut arch_manager = ArchManager::new();
    arch_manager.init();

    // setup system table singleton
    SYSTEM_TABLE.store((&mut system_table).as_ptr().cast_mut(), Ordering::Release);

    // Get boot services
    let boot_services = system_table.boot_services();

    unsafe {
        // setup logging
        init_logging(&mut get_system_table());

        //setup memory allocator
        allocator::init(system_table.boot_services());

        // Schedule a event to prevent the EFI tools to be used after exit
        boot_services
            .create_event(
                EventType::SIGNAL_EXIT_BOOT_SERVICES,
                Tpl::NOTIFY,
                Some(exit_boot_services),
                None,
            )
            .map(Some);
    }

    // Reset the console before continue
    get_system_table()
        .stdout()
        .reset(false)
        .expect("Failed to reset stdout");

    check_revision(system_table.uefi_revision());

    // initialize command manager
    command_manager::init();

    // Firmware is required to set a 5 minute watchdog timer before running an image. Disable it.
    boot_services
        .set_watchdog_timer(0, 0x10000, None)
        .expect("efi: could not disable watchdog timer");

    // Initialize memory manager
    let memory_manager = MemoryManager::new();
    memory_manager.init(&boot_services);

    // Get the loader image protocol
    let loader_image = boot_services
        .open_protocol_exclusive::<LoadedImage>(image_handle)
        .expect("efi: failed to retrieve `LoaderImage` protocol from handle");

    unsafe { LOADED_IMAGE = NonNull::new(&loader_image as *const _ as *mut _) };

    // save image handle
    unsafe { IMAGE_HANDLE = NonNull::new(&image_handle as *const _ as *mut _) };

    // initialize the video system
    EFIVideoManager::init();

    // Create a console manager
    ConsoleOutManager::init();
    ConsoleInDevice::init();

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

unsafe extern "efiapi" fn exit_boot_services(_e: Event, _ctx: Option<NonNull<c_void>>) {
    SYSTEM_TABLE.store(ptr::null_mut(), Ordering::Release);

    LOGGER.disable();
}

/// Reboot the system
pub fn target_reboot() -> ! {
    get_system_table().runtime_services().reset(
        uefi::table::runtime::ResetType::WARM,
        Status::SUCCESS,
        None,
    );
}
