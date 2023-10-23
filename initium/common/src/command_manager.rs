use alloc::string::String;
use alloc::vec::Vec;

/// static reference for the command manager
pub static mut COMMAND_MANAGER: Option<CommandManager<'static>> = None;

/// Type for a command function
pub type CommandFn = fn(Vec<String>) -> bool;

/// Initialize the command manager
pub fn init() {
    let manager = CommandManager {
        commands: Vec::new(),
    };
    unsafe { COMMAND_MANAGER = Some(manager) };
}

/// Get command manager reference
pub fn get_command_manager<'a>() -> &'a mut CommandManager<'static> {
    unsafe {
        let option = &mut COMMAND_MANAGER;
        option
            .as_mut()
            .expect("Commands manager must be initialized first")
    }
}

/// Structure for the builtin command
pub struct BuiltinCommand<'a> {
    /// Name of the command
    pub name: &'a str,
    /// Description of the command
    pub description: &'a str,
    /// Command function
    pub func: CommandFn,
}

/// Manager for the builtin commands
pub struct CommandManager<'a> {
    commands: Vec<BuiltinCommand<'a>>,
}

impl<'a> CommandManager<'a> {
    /// Add a new command
    pub fn add_command(&mut self, command: BuiltinCommand<'a>) {
        self.commands.push(command);
    }

    pub fn get_command(&self, command_name: &String) -> Option<&BuiltinCommand> {
        self.commands.iter().find(|&c| c.name == command_name)
    }

    pub fn get_commands(&self) -> &Vec<BuiltinCommand> {
        &self.commands
    }
}
