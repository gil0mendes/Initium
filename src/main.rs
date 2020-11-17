//! Initium is a modern bootloader with no legacy components.

#![no_std]
#![no_main]

#![feature(panic_info_message)]

extern crate alloc;
extern crate platform;
#[macro_use]
extern crate log;
extern crate common;

mod config;

use alloc::boxed::Box;
use platform::platform_manager;

#[no_mangle]
pub extern fn load_main() {
    unsafe {
        let platform_manager = platform_manager();

        // init console
        platform_manager.as_ref().console_manager().as_mut().init();

        let mode = platform_manager.as_ref().video_manager().as_ref().get_mode();
        info!("resolution: {}x{}", mode.width, mode.height);

        // platform_manager.as_ref().console_manager().as_mut().clear(0, 0, 0, 0);
    }

    loop {}
}

#[panic_handler]
fn panic_handler(info: &core::panic::PanicInfo) -> ! {
    if let Some(location) = info.location() {
        error!(
            "Panic in {} at ({}, {}):",
            location.file(),
            location.line(),
            location.column()
        );
        if let Some(message) = info.message() {
            error!("{}", message);
        }
    }

    loop {}
}
