#![no_std]
#![no_main]

#![feature(alloc)]
#![feature(compiler_builtins_lib)]
#![feature(panic_implementation)]
#![feature(try_trait)]

extern crate uefi;

#[macro_use]
extern crate log;
#[macro_use]
extern crate alloc;

use core::panic::PanicInfo;
use core::ptr;

use uefi::prelude::*;

use self::memory::MemoryManager;
use self::video::VideoManager;

mod memory;
mod video;

extern {
    fn load_main();
}

/// Check if the UEFI where we are running on is compatible
/// with the loader.
fn check_revision(rev: uefi::table::Revision) {
    let (major, minor) = (rev.major(), rev.minor());

    info!("UEFI {}.{}", major, minor);
    
    assert!(major >= 2, "Running on an old, unsupported version of UEFI");
    assert!(minor >= 30, "Old version of UEFI 2, some features might not be available.");
}

/// Entry point for EFI platforms
#[no_mangle]
pub extern "C" fn uefi_start(_image_handle: uefi::Handle, system_table: &'static SystemTable) -> Status {
    // Initialize logging.
    uefi_services::init(system_table);

    check_revision(system_table.uefi_revision());

    // Firmware is required to set a 5 minute watchdog timer before
	// running an image. Disable it.
    system_table.boot.set_watchdog_timer(0, 0x10000, None)
        .expect("Could not disable watchdog timer");

    // Initialize memory manager
    let memoryManager = MemoryManager::new();
    memoryManager.init(&system_table.boot);

    // Initialize video manager
    let mut videoManager = VideoManager::new();
    videoManager.init(&system_table.boot);

    // Call loader main function
    unsafe { load_main(); }

    Status::Success
}
