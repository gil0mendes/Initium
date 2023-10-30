use crate::print;
use alloc::string::String;
use common::{console::ConsoleOut, key::Key};

use super::{list_window::Entry, render_help_action};

pub struct ChoiceEntry {
    pub label: String,
}

impl Entry for ChoiceEntry {
    fn render(&self, _console: &dyn ConsoleOut) {
        print!("{}", self.label);
    }

    fn help(&self, console: &dyn ConsoleOut) {
        render_help_action(console, Key::Printable('\n'), "Select");

        // render help for shell
        render_help_action(
            console,
            Key::Special(common::key::ScanCode::FUNCTION_2),
            "Shell",
        );
    }
}
