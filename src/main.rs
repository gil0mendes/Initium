//! Initium is a modern bootloader with no legacy components.

#![no_std]
#![no_main]
#![feature(panic_info_message)]

use core::{borrow::Borrow, cell::Cell};

use alloc::{boxed::Box, string::String, vec::Vec};
use console::{get_console_in, get_console_out};
use ui::{ChoiceEntry, ListWindow};

use common::{
    command_manager::get_command_manager,
    key::{Key, ScanCode},
    BuiltinCommand,
};

extern crate alloc;
#[macro_use]
extern crate platform;
#[macro_use]
extern crate log;
extern crate common;

mod config;
mod console;
mod line_editor;
mod shell;
mod ui;

#[no_mangle]
pub extern "C" fn load_main() {
    use crate::alloc::string::ToString;

    // init console
    get_console_out().init();

    register_commands();

    // TODO: this is a simple test is to be removed on the future
    let mut window = ListWindow::new("Boot Menu".to_string(), false);

    // create a list of entries
    window.add_list(
        Box::new(ChoiceEntry {
            label: "Example OS Choice".to_string(),
        }),
        false,
    );
    window.add_list(
        Box::new(ChoiceEntry {
            label: "Example OS Choice 2".to_string(),
        }),
        false,
    );
    window.add_list(
        Box::new(ChoiceEntry {
            label: "Example OS Choice 3".to_string(),
        }),
        false,
    );

    {
        let console = get_console_out();
        window.render(console, 0);
    }

    // TODO: remove this as soon as we have the environment loader implemented
    loop {}
}

/// Register about command
fn register_commands() {
    let command_manager = get_command_manager();

    command_manager.add_command(BuiltinCommand {
        name: "about",
        description: "Shows the bootloader version",
        func: builtin_about,
    });

    // we can't implement the 'help' command on the common crate since the println! macro isn't available there
    command_manager.add_command(BuiltinCommand {
        name: "help",
        description: "List all available commands",
        func: help_command,
    });
}

fn builtin_about(_: Vec<String>) -> bool {
    println!("Initium version {}", env!("CARGO_PKG_VERSION"));
    true
}

fn help_command(_: Vec<String>) -> bool {
    let manager = get_command_manager();

    println!("Command          Description");
    println!("-------          -----------");

    manager.get_commands().iter().for_each(|c| {
        println!("{:<16} {}", &c.name, &c.description);
    });

    true
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

    // TODO: halt in a way that makes sense for the platform
    loop {}
}
