///! User Interface
mod choice_entry;
mod list_window;

use alloc::string::String;
use common::console::ConsoleOut;
use list_window::Entry;

pub use self::choice_entry::ChoiceEntry;
pub use self::list_window::ListWindow;

/// Return codes for input handling functions
pub enum InputResult {
    /// No special acton needed
    Handled,
    /// Re-render the list entry
    RenderEntry,
    /// Re-render help (in case possible actions change)
    RenderHelp,
    /// Re-render the whole window
    RenderWindow,
    /// Close the window
    Close,
}

/// Render helper action
pub fn render_help_action(console: &ConsoleOut, key: char, name: &str) {
    use crate::alloc::string::ToString;

    let label = match key {
        '\n' => "Enter".to_string(),
        _ => key.to_string(),
    };

    print!("{} = {} ", label, name);
}
