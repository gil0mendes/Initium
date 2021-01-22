use common::key::Key;

use crate::{
    console::{get_console_in, get_console_out},
    line_editor::LineEditor,
};

pub struct Shell {}

impl Shell {
    /// Create a new shell instance
    pub fn new() -> Self {
        Self {}
    }

    /// Shell entry point
    ///
    ///
    pub fn start(&mut self) -> ! {
        let mut console_out = get_console_out();

        // reset the region and clear the console
        console_out.reset_region();
        console_out.clear(0, 0, 0, 0);

        // reset console color
        console_out.set_color(common::console::Color::White, common::console::Color::Black);

        loop {
            print!("Initium> ");

            self.input_handler();
        }
    }

    fn input_handler(&mut self) {
        let mut line_editor = LineEditor::new();

        let mut console_in = get_console_in();

        // Accumulate another line
        loop {
            let key = console_in.get_char();

            line_editor.handle_input(key);
        }
    }
}
