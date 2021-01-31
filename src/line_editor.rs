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
    pub fn reprint_from_current(&self, print_space: bool) {
        print!("{}", &self.buffer[self.offset..]);

        // print an additional space at the end when remove a character
        if print_space {
            print!(" \x08");
        }

        // Put the cursor back on the previous position. When we print to the console it will move the cursor forward.
        for _ in self.offset..self.buffer.len() {
            print!("\x08");
        }
    }

    /// Handle input on the line editor.
    pub fn handle_input(&mut self, key: Key) {
        match key {
            Key::Special(common::key::ScanCode::LEFT) => {
                if self.offset > 0 {
                    print!("\x08");
                    self.offset = self.offset - 1;
                }
            }
            Key::Special(common::key::ScanCode::RIGHT) => {
                if self.offset != self.buffer.len() {
                    print!("{}", self.buffer.chars().nth(self.offset).unwrap());
                    self.offset = self.offset + 1;
                }
            }
            Key::Special(common::key::ScanCode::HOME) => {
                while self.offset > 0 {
                    print!("\x08");
                    self.offset = self.offset - 1;
                }
            }
            Key::Special(common::key::ScanCode::END) => {
                while self.offset != self.buffer.len() {
                    print!("{}", self.buffer.chars().nth(self.offset).unwrap());
                    self.offset = self.offset + 1;
                }
            }
            // Backspace
            Key::Printable('\x08') => {
                self.erase_char(false);
            }
            // Del
            Key::Printable('\x78') => {
                self.erase_char(true);
            }
            Key::Printable('\n') => {
                self.offset = self.buffer.len();
                self.insert_char('\n');
            }
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
            self.reprint_from_current(false);
        }
    }

    /// Erase a character from the current position.
    ///
    /// # Arguments
    /// - `forward`: If true, will erase the character at the current cursor position, else will erase the previous one.
    fn erase_char(&mut self, forward: bool) {
        if forward && self.offset == self.buffer.len() {
            // when is at the end of the line
            return;
        } else if !forward && self.offset == 0 {
            // when there is nothing to remove back
            return;
        } else if !forward {
            // when is a normal backspace
            print!("\x08");
            self.offset = self.offset - 1;
        }

        self.buffer.remove(self.offset);

        self.reprint_from_current(true);
    }
}
