use core::{
    fmt, ptr,
    sync::atomic::{AtomicPtr, Ordering},
};

use crate::platform;
use common::console::{Color, ConsoleIn, ConsoleOut, Cursor, DrawRegion};

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
    use core::fmt::Write;

    // TODO: implement
    return Ok(());

    // unsafe {
    //     let mut console = CONSOLE_OUT.expect("The platform console was not initialized");
    //     console.as_mut().write_fmt(args)
    // }
}

/// Reference for the console manager
static mut CONSOLE_MANAGER: ConsoleManager = ConsoleManager::new();

pub struct ConsoleManager {
    primary_console: AtomicPtr<&'static mut dyn ConsoleOut>,
    debug_console: AtomicPtr<&'static mut dyn ConsoleOut>,
}

impl ConsoleManager {
    /// Const method to initialize the console manager code
    pub const fn new() -> Self {
        Self {
            primary_console: AtomicPtr::new(ptr::null_mut()),
            debug_console: AtomicPtr::new(ptr::null_mut()),
        }
    }

    pub fn primary_console(&self) -> Option<&'static mut dyn ConsoleOut> {
        let console = self.primary_console.load(Ordering::Acquire);

        if console.is_null() {
            None
        } else {
            unsafe { Some(*console) }
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
    loop {}
    // TODO: implement
    // unsafe {
    //     let mut console_option = platform::CONSOLE_OUT.expect("platform console not initialized");
    //     console_option.as_mut()
    // }
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
    let console = console_manager().primary_console().unwrap();

    let (width, height) = console.resolution();

    // reset state and clear the default colors
    console.set_region(DrawRegion {
        x: 0,
        y: 0,
        width,
        height,
        scrollable: false,
    });
    console.set_cursor(Cursor::new(0, 0, true));
    console.set_color(Color::White, Color::Black);
    console.clear(0, 0, 0, 0);
}
