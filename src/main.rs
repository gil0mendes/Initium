//! Initium is a modern bootloader with no legacy components.

#![no_std]
#![no_main]
#![feature(panic_info_message)]

use ui::Window;

extern crate alloc;
extern crate platform;
#[macro_use]
extern crate log;
extern crate common;

mod config;
mod ui;

#[no_mangle]
pub extern "C" fn load_main() {
    use crate::alloc::string::ToString;

    unsafe {
        // init console
        use common::video::ConsoleOut;
        let mut consoleOption = &mut platform::CONSOLE_MANAGER;
        let mut console = consoleOption.as_mut().unwrap();

        console.init();

        // TODO: this is a simple test is to be removed on the future
        let window = Window::new("example".to_string());
        window.render(console);
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
