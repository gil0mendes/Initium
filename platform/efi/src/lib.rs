#![no_std]
#![no_main]

#![feature(asm)]
#![feature(alloc)]
#![feature(try_trait)]
#![feature(tool_lints)]
#![feature(abi_efiapi)]
#![feature(const_mut_refs)]

// Keep this line to ensure the `mem*` functions are linked in.
extern crate rlibc;

extern crate uefi;

#[macro_use]
extern crate log;
extern crate alloc;
extern crate arch;

use uefi::prelude::*;
use uefi::Status;

use self::memory::MemoryManager;
use self::video::VideoManager;
use arch::ArchManager;

mod disk;
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
pub extern "C" fn efi_main(_image_handle: uefi::Handle, system_table: SystemTable<Boot>) -> Status {
    // Initialize arch code
    let mut arch_manager = ArchManager::new();
    arch_manager.init();

    // Initialize logging.
    uefi_services::init(&system_table).expect_success("Failed to initialize utilities");

    // Reset the console before continue
    system_table.stdout().reset(false).expect_success("Failed to reset stdout");

    check_revision(system_table.uefi_revision());

    // Get boot services
    let boot_services = system_table.boot_services();

    // Firmware is required to set a 5 minute watchdog timer before
	// running an image. Disable it.
    boot_services.set_watchdog_timer(0, 0x10000, None)
        .expect("Could not disable watchdog timer");

    // Initialize memory manager
    let memory_manager = MemoryManager::new();
    memory_manager.init(&boot_services);

    // Initialize video manager
    let mut video_manager = VideoManager::new();
    video_manager.init(&boot_services);

    /*test_fs(&system_table.boot);*/

    info!("internal time: {}", arch_manager.time_manager.current_time());

    // Call loader main function
    unsafe { load_main(); }

    Status::SUCCESS
}
