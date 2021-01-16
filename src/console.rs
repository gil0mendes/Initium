use common::console::{ConsoleIn, ConsoleOut};

/// Get borrow reference for console output
pub fn get_console_out<'a>() -> &'a mut dyn ConsoleOut {
    unsafe {
        let mut consoleOption = &mut platform::CONSOLE_OUT;
        consoleOption
            .as_mut()
            .expect("Console does not support output capability")
    }
}

/// Get borrow reference for console input
pub fn get_console_in<'a>() -> &'a mut dyn ConsoleIn {
    unsafe {
        let mut consoleOption = &mut platform::CONSOLE_IN;
        consoleOption
            .as_mut()
            .expect("Console does not support input capability")
    }
}
