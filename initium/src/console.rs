use core::fmt::{self, Write};

use crate::platform;
use alloc::{boxed::Box, string};
use common::console::{Color, ConsoleIn, ConsoleOut, Cursor};

/// Prints to the standard output.
///
/// # Examples
/// ```
/// print!("");
/// print!("Hello World\n");
/// print!("Hello {}", "World");
/// ```
#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => (
        $crate::console::print_fmt(core::format_args!($($arg)*)).unwrap();
    );
}

/// Print to the standard output with a new line.
///
/// # Examples
/// ```
/// println!();
/// println!("Hello World");
/// println!("Hello {}", "World");
/// ```
#[macro_export]
macro_rules! println {
    () => ($crate::platform::console::print!("\n"));
    ($($arg:tt)*) => ($crate::console::print_fmt(core::format_args!("{}{}", core::format_args!($($arg)*), "\n")));
}

pub fn print_fmt(args: fmt::Arguments) -> fmt::Result {
    let primary_console = console_manager().primary_console.as_mut();

    if primary_console.is_none() {
        return Ok(());
    }

    let console = primary_console.unwrap();
    console.as_mut().write_fmt(args)
}

/// Reference for the console manager
static mut CONSOLE_MANAGER: ConsoleManager = ConsoleManager::new();

pub struct ConsoleManager {
    pub primary_console: Option<Box<dyn ConsoleOut>>,
    pub debug_console: Option<Box<dyn ConsoleOut>>,
}

impl ConsoleManager {
    /// Const method to initialize the console manager code
    pub const fn new() -> Self {
        Self {
            primary_console: None,
            debug_console: None,
        }
    }

    pub fn primary_console(&mut self) -> Option<&mut dyn ConsoleOut> {
        match self.primary_console {
            Some(ref mut console) => Some(console.as_mut()),
            None => None,
        }
    }

    pub fn set_primary_console(&mut self, console: Option<Box<dyn ConsoleOut>>) {
        match console {
            Some(ref con) => {
                self.primary_console = console;
            }
            None => {
                self.primary_console = None;
            }
        }
    }
}

/// Initialize common console
pub fn init() {
    platform::console::init();
    // TODO: set primary console
}

/// Get the console manager
pub fn console_manager() -> &'static mut ConsoleManager {
    unsafe { &mut CONSOLE_MANAGER }
}

/// Get borrow reference for console output
pub fn get_console_out<'a>() -> &'a mut dyn ConsoleOut {
    console_manager().primary_console().unwrap()
}

/// Get borrow reference for console input
pub fn get_console_in<'a>() -> &'a mut dyn ConsoleIn {
    unsafe {
        let console_option = &mut platform::CONSOLE_IN;
        console_option
            .as_mut()
            .expect("Console does not support input capability")
    }
}

/// End UI mode reset the console.
pub fn console_end_ui() {
    // TODO: check if we are on UI mode
    let console = get_console_out();

    // reset state and clear the default colors
    console.reset_region();
    console.set_cursor(Cursor::new(0, 0, true));
    console.set_color(Color::White, Color::Black);
    console.clear(0, 0, 0, 0);
}
