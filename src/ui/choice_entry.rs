use alloc::string::String;
use common::console::ConsoleOut;

use super::{list_window::Entry, render_help_action};

pub struct ChoiceEntry {
    pub label: String,
}

impl Entry for ChoiceEntry {
    fn render(&self, console: &ConsoleOut) {
        print!("{}", self.label);
    }

    fn help(&self, console: &ConsoleOut) {
        render_help_action(console, '\n', "Select");
    }
}
