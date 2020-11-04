//! Initium is a modern bootloader with no legacy components.

#![no_std]
#![no_main]

#![feature(panic_info_message)]

extern crate alloc;
extern crate platform;
#[macro_use]
extern crate log;

mod config;

use alloc::boxed::Box;

#[no_mangle]
pub extern fn load_main() {
    let heap_example = Box::new(1993);
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
