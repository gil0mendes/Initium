use alloc::{
    string::{String, ToString},
    vec::Vec,
};

use common::{command_manager::get_command_manager, key::Key};

use crate::{
    console::{get_console_in, get_console_out},
    line_editor::LineEditor,
    print, println,
};

/// Implementation of the bootloader shell.
///
/// This allows to execute commands on a shell and get information about the environment.
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
        let console_out = get_console_out();

        // reset the region and clear the console
        console_out.reset_region();
        console_out.clear(0, 0, 0, 0);

        // reset console color
        console_out.set_color(common::console::Color::White, common::console::Color::Black);

        loop {
            print!("Initium> ");

            // get a command from the user and process it.
            let raw_command = self.input_handler();
            let command = self.process_command(raw_command);

            // ignore when the command is empty
            if command.name.len() == 0 {
                continue;
            }

            self.execute_command(command);
        }
    }

    /// Process the command input and return a Command structure
    fn process_command(&self, raw_command: String) -> Command {
        let parts: Vec<&str> = raw_command.trim().split(" ").collect();
        let command_name = parts.first().unwrap().to_string();
        let args: Vec<String> = parts.iter().skip(1).map(|&arg| arg.to_string()).collect();

        Command {
            name: command_name,
            arguments: args,
        }
    }

    /// Execute the given command.
    fn execute_command(&self, list: Command) {
        let mut command_manager = get_command_manager();

        match command_manager.get_command(&list.name) {
            Some(command) => {
                (command.func)(list.arguments);
            }
            None => {
                println!("Command '{}' not found", &list.name);
            }
        }
    }

    /// Handle the user input
    fn input_handler(&mut self) -> String {
        let mut line_editor = LineEditor::new();

        let console_in = get_console_in();

        // Accumulate another line
        loop {
            let key = console_in.get_char();

            line_editor.handle_input(key);

            // when the user hit an enter return the collected line
            match key {
                Key::Printable('\n') => return line_editor.get_line(),
                _ => {}
            }
        }
    }
}

/// Structure containing details of a command to run.
#[derive(Debug)]
struct Command {
    /// Name of the command
    name: String,
    /// List of arguments
    arguments: Vec<String>,
}
