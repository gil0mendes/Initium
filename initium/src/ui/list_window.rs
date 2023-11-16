use crate::{
    console::{console_end_ui, get_console_in},
    shell::Shell,
};

use super::InputResult;
use crate::print;
use alloc::boxed::Box;
use alloc::string::String;
use alloc::vec::Vec;
use common::console::{Color, DrawRegion};
use common::console::{ConsoleOut, Cursor};

/// Trait to be implemented by a UI list entry
pub trait Entry {
    /// Render the entry
    fn render(&self, console: &dyn ConsoleOut);

    /// Write the help text for the entry
    fn help(&self, console: &dyn ConsoleOut);

    /// Destroy the entry
    fn destroy(&self) {}

    /// Handle input on the entry
    fn input(&mut self, _key: u16) -> InputResult {
        InputResult::Handled
    }
}

/// Set the draw region to the content region
#[inline]
fn set_content_region(console: &mut dyn ConsoleOut) {
    let (width, height) = console.resolution();

    let region = DrawRegion {
        x: 2,
        y: 3,
        width,
        height,
        scrollable: false,
    };

    console.set_region(region);
    console.set_color(Color::LightGrey, Color::Black);
}

/// Structure defining a window type
pub struct ListWindow {
    /// Title of the Window
    title: String,

    /// Whether the menu can be exited
    exitable: bool,
    /// Array of entries
    entries: Vec<Box<dyn Entry>>,
    /// Index of selected entry
    selected: usize,
}

impl ListWindow {
    /// Create a new list window
    pub fn new(title: String, exitable: bool) -> Self {
        Self {
            title,
            exitable,
            entries: Vec::<Box<dyn Entry>>::new(),
            selected: 0,
        }
    }

    /// Set the draw region to the title region
    #[inline]
    fn set_title_region(&self, console: &mut dyn ConsoleOut) {
        let (width, _) = console.resolution();

        let region = DrawRegion {
            x: 2,
            y: 1,
            width: width - 4,
            height: 1,
            scrollable: false,
        };

        console.set_region(region);
        console.set_color(Color::White, Color::Black);
    }

    /// Set the draw region to the content region
    #[inline]
    fn set_content_region(&self, console: &mut dyn ConsoleOut) {
        let (console_width, console_height) = console.resolution();
        let (content_width, content_height) = (console_width - 4, console_height - 6);

        let region = DrawRegion {
            x: 2,
            y: 3,
            width: content_width,
            height: content_height,
            scrollable: false,
        };

        console.set_region(region);
        console.set_color(Color::LightGrey, Color::Black);
    }

    #[inline]
    fn set_help_region(&self, console: &mut dyn ConsoleOut) {
        let (console_width, console_height) = console.resolution();
        let (help_width, help_height) = (console_width - 4, 1);

        let region = DrawRegion {
            x: 2,
            y: console_height - 2,
            width: help_width,
            height: help_height,
            scrollable: false,
        };

        console.set_region(region);
        console.set_color(Color::White, Color::Black);
    }

    /// Render help text for a windows
    fn render_help(&self, console: &mut dyn ConsoleOut, _timeout: usize, _update: bool) {
        self.set_help_region(console);

        // render the helper text for the selected component
        match self.entries.get(self.selected) {
            Some(entry) => entry.help(console),
            _ => {}
        };

        // TODO: render back action if is list is exitable
    }

    /// Render an entry from a list
    fn render_entry(
        &self,
        console: &mut dyn ConsoleOut,
        entry: &dyn Entry,
        pos: usize,
        selected: bool,
    ) {
        let content_region = console.get_region();

        // compute the place where to put the entry
        let region = DrawRegion {
            x: content_region.x,
            y: content_region.y + pos,
            width: content_region.width,
            height: 1,
            scrollable: false,
        };
        console.set_region(region);

        // clear the area. if there is a entry is selected it should be highlighted
        let (fg_color, bg_color) = match selected {
            true => (Color::Black, Color::LightGrey),
            false => (Color::LightGrey, Color::Black),
        };
        console.set_color(fg_color, bg_color);
        console.clear(0, 0, 0, 0);

        // render the entry
        entry.render(console);

        // restore content region and color
        console.set_region(content_region);
        console.set_color(Color::LightGrey, Color::Black);
    }

    /// Render contents of a window
    pub fn render(&mut self, console: &mut dyn ConsoleOut, timeout: usize) {
        set_content_region(console);
        console.set_color(Color::White, Color::Black);
        console.clear(0, 0, 0, 0);

        // disable the cursor
        console.set_cursor(Cursor::default());

        // draw the title
        self.set_title_region(console);
        print!("{}", self.title);

        // draw help text
        self.render_help(console, timeout, false);

        // draw content last, so console state set by render is preserved
        self.set_content_region(console);

        // render all list entries
        self.entries.iter().enumerate().for_each(|(pos, entry)| {
            let is_selected = pos == self.selected;
            self.render_entry(console, entry.as_ref(), pos, is_selected);
        });

        // TODO: abstract this to be used with all UI components
        loop {
            let key = get_console_in().get_char();

            match key {
                // move selection up
                common::key::Key::Special(common::key::ScanCode::UP) => {
                    let target = self.selected;

                    // move selection to bottom if is the first entry, for now prevent navigation to top
                    self.selected = if self.selected == 0 {
                        self.entries.len() - 1
                    } else {
                        self.selected - 1
                    };

                    // render the previous selected entry
                    self.render_entry(
                        console,
                        self.entries.get(target).unwrap().as_ref(),
                        target,
                        false,
                    );

                    // render the next entry as select
                    self.render_entry(
                        console,
                        self.entries.get(self.selected).unwrap().as_ref(),
                        self.selected,
                        true,
                    );
                }
                // move selection down
                common::key::Key::Special(common::key::ScanCode::DOWN) => {
                    let target = self.selected;
                    // move selection to top if is the last entry, for now prevent navigation to bottom
                    self.selected = if self.selected == self.entries.len() - 1 {
                        0
                    } else {
                        self.selected + 1
                    };

                    // render the previous selected entry
                    self.render_entry(
                        console,
                        self.entries.get(target).unwrap().as_ref(),
                        target,
                        false,
                    );

                    // render the next entry as select
                    self.render_entry(
                        console,
                        self.entries.get(self.selected).unwrap().as_ref(),
                        self.selected,
                        true,
                    );
                }
                // enter on shell
                common::key::Key::Special(common::key::ScanCode::FUNCTION_2) => {
                    // end ui mode
                    console_end_ui();

                    let mut shell = Shell::new();
                    shell.start();
                }
                _ => {}
            }
        }
    }

    pub fn add_list(&mut self, entry: Box<dyn Entry>, _selected: bool) {
        self.entries.push(entry);

        // TODO: implement selected logic
    }
}
