use core::usize;

///! Line editor to be used on the shell.
use alloc::string::String;
use common::{console::ConsoleOut, key::Key};

use crate::console::get_console_out;

/// Initial size of the allocated string.
const DEFAULT_CAPACITY: usize = 128;

/// Check if the char is printable
fn is_printable_char(ch: char) -> bool {
    ch.is_alphanumeric() || ch.is_whitespace()
}

/// Line editor state
pub struct LineEditor {
    /// String being edited
    buffer: String,
    /// Current string offset
    offset: usize,
}

impl LineEditor {
    /// Create a new line editor.
    ///
    /// By default this starts with a empty string but with some memory allocated to prevent constant reallocations.
    pub fn new() -> Self {
        Self {
            buffer: String::with_capacity(DEFAULT_CAPACITY),
            offset: 0,
        }
    }

    /// Output the line and place the cursor at the current position.
    pub fn output(&self, console: &dyn ConsoleOut) {
        unimplemented!();
    }

    /// Reprint from the current position offset, maintaining the cursor position.
    pub fn reprint_from_current(&self) {
        unimplemented!();
    }

    /// Handle input on the line editor.
    pub fn handle_input(&mut self, key: Key) {
        match key {
            Key::Printable(ch) => {
                if is_printable_char(ch) {
                    self.insert_char(ch);
                }
            }
            _ => {}
        }
    }

    /// Allows to read the current line string
    pub fn get_line(&self) -> String {
        self.buffer.clone()
    }

    /// Insert a new character to the bugger at the current position
    fn insert_char(&mut self, ch: char) {
        print!("{}", ch);

        if self.offset == self.buffer.len() {
            self.buffer.push(ch);
            self.offset = self.offset + 1;
        } else {
            self.buffer.insert(self.offset, ch);
            self.offset = self.offset + 1;

            // Reprint the character plus everything after.
            self.reprint_from_current();
        }
    }
}
