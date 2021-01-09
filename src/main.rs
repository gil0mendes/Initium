//! Initium is a modern bootloader with no legacy components.

#![no_std]
#![no_main]
#![feature(panic_info_message)]

use alloc::boxed::Box;
use common::console::ConsoleIn;
use ui::{ChoiceEntry, ListWindow};

extern crate alloc;
#[macro_use]
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
        use common::console::ConsoleOut;
        let mut consoleOption = &mut platform::CONSOLE_OUT;
        let mut console = consoleOption.as_mut().unwrap();

        console.init();

        // TODO: this is a simple test is to be removed on the future
        let mut window = ListWindow::new("Boot Menu".to_string(), false);

        // create a list of entries
        window.add_list(
            Box::new(ChoiceEntry {
                label: "Example OS Choice".to_string(),
            }),
            false,
        );

        window.render(console, 0);

        {
            use ConsoleIn;
            let mut consoleInOption = &mut platform::CONSOLE_IN;
            let mut input = consoleInOption.as_mut().unwrap();

            let key = input.get_char() as u8 as char;
            print!(">>> '{}'", key);
        }
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
