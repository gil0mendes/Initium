use common::console::{Color, ConsoleIn, ConsoleOut};

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

/// End UI mode reset the console.
pub fn console_end_ui() {
    // TODO: check if we are on UI mode
    let mut console = get_console_out();

    // reset state and clear the default colors
    console.reset_region();
    console.set_cursor(0, 0, true);
    console.set_color(Color::White, Color::Black);
    console.clear(0, 0, 0, 0);
}
