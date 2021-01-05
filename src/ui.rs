///! User Interface
use alloc::string::String;
use common::console::Color;
use common::video::ConsoleOut;

/// Structure defining a window type
pub struct Window {
    /// Title of the Window
    title: String,
}

impl Window {
    /// Create a new Window
    pub fn new(title: String) -> Self {
        Self { title }
    }

    /// Render contents of a window
    pub fn render(&self, console: &mut dyn ConsoleOut) {
        console.reset_region();
        console.set_color(Color::White, Color::Black);
        console.clear(0, 0, 0, 0);
        /*

        - disable cursor

        - draw title

        - render help text

        - set new draw region
        - render window based on type
        */
    }
}
