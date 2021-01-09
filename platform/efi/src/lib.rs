#![no_std]
#![feature(asm)]
#![feature(try_trait)]
#![feature(abi_efiapi)]
#![feature(const_mut_refs)]
#![feature(alloc_error_handler)]
#![feature(in_band_lifetimes)]
#![feature(num_as_ne_bytes)]

// Keep this line to ensure the `mem*` functions are linked in.
extern crate rlibc;

extern crate uefi;

#[macro_use]
extern crate log;
extern crate alloc;
extern crate arch;
pub mod allocator;
extern crate common;
extern crate spin;

use uefi::prelude::*;
use uefi::Status;

use self::memory::MemoryManager;
use self::video::EFIVideoManager;
use crate::console::{ConsoleInDevice, ConsoleOutManager};
use arch::ArchManager;
use uefi::table::SystemTable;

pub mod console;
mod disk;
mod memory;
mod video;

pub use console::{CONSOLE_IN, CONSOLE_OUT};
pub use video::VIDEO_MANAGER;

extern "C" {
    fn load_main();
}

/// Reference to the system table.
///
/// This table is only fully safe to use until UEFI boot services have been exited.
/// After that, some fields and methods are unsafe to use, see the documentation of
/// UEFI's ExitBootServices entry point for more details.
static mut SYSTEM_TABLE: Option<SystemTable<Boot>> = None;

/// Global logger object
static mut LOGGER: Option<uefi::logger::Logger> = None;

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
pub extern "C" fn efi_main(_image_handle: uefi::Handle, system_table: SystemTable<Boot>) -> Status {
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

    // Get boot services
    let boot_services = system_table.boot_services();

    // Firmware is required to set a 5 minute watchdog timer before
    // running an image. Disable it.
    boot_services
        .set_watchdog_timer(0, 0x10000, None)
        .expect("Could not disable watchdog timer");

    // Initialize memory manager
    let memory_manager = MemoryManager::new();
    memory_manager.init(&boot_services);

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
    unsafe {
        load_main();
    }

    Status::SUCCESS
}
