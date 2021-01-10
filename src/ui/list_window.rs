use super::InputResult;
use alloc::boxed::Box;
use alloc::string::String;
use alloc::vec::Vec;
use common::console::ConsoleOut;
use common::console::{Color, DrawRegion};

/// Trait to be implemented by a UI list entry
pub trait Entry {
    /// Render the entry
    fn render(&self, console: &ConsoleOut);

    /// Write the help text for the entry
    fn help(&self, console: &ConsoleOut);

    /// Destroy the entry
    fn destroy(&self) {}

    /// Handle input on the entry
    fn input(&mut self, key: u16) -> InputResult {
        InputResult::Handled
    }
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
            entries: Vec::<Box<Entry>>::new(),
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
        console.set_color(Color::Light_grey, Color::Black);
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
    fn render_help(&self, console: &mut dyn ConsoleOut, timeout: usize, update: bool) {
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
        entry: &Entry,
        pos: usize,
        selected: bool,
    ) {
        let (console_w, console_h) = console.resolution();

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
            true => (Color::Black, Color::Light_grey),
            false => (Color::Light_grey, Color::Black),
        };
        console.set_color(fg_color, bg_color);
        console.clear(0, 0, 0, 0);

        // render the entry
        entry.render(console);

        // restore content region and color
        console.set_region(content_region);
        console.set_color(Color::Light_grey, Color::Black);
    }

    /// Render contents of a window
    pub fn render(&self, console: &mut dyn ConsoleOut, timeout: usize) {
        console.reset_region();
        console.set_color(Color::White, Color::Black);
        console.clear(0, 0, 0, 0);

        // disable the cursor
        console.set_cursor(0, 0, false);

        // draw the title
        self.set_title_region(console);
        print!("{}", self.title);

        // draw help text
        self.render_help(console, timeout, false);

        // draw content last, so console state set by render is preserved
        self.set_content_region(console);

        /// render all list entries
        self.entries.iter().enumerate().for_each(|(pos, entry)| {
            let is_selected = pos == self.selected;
            self.render_entry(console, entry.as_ref(), pos, is_selected);
        });
    }

    pub fn add_list(&mut self, entry: Box<Entry>, selected: bool) {
        self.entries.push(entry);

        // TODO: implement selected logic
    }
}
