///! User Interface
mod choice_entry;
mod list_window;

use crate::print;
use common::{console::ConsoleOut, key::Key};

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
pub fn render_help_action(console: &dyn ConsoleOut, key: Key, name: &str) {
    use crate::alloc::string::ToString;

    let label = match key {
        Key::Printable(ch) => match ch {
            '\n' => "Enter".to_string(),
            _ => ch.to_string(),
        },
        Key::Special(common::key::ScanCode::FUNCTION_2) => "F2".to_string(),
        other => unimplemented!("Implement print for {:?}", other),
    };

    print!("{} = {} ", label, name);
}
