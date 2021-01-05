///! User Interface
use alloc::string::String;
use common::console::{Color, DrawRegion};
use common::video::ConsoleOut;

/// Structure defining a window type
pub struct Window {
    /// Title of the Window
    title: String,
}

impl Window {
    /// Create a new Window
    pub fn new(title: String) -> Self {
        Self { title }
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
        // TODO: set color
    }

    /// Render contents of a window
    pub fn render(&self, console: &mut dyn ConsoleOut) {
        console.reset_region();
        console.set_color(Color::White, Color::Black);
        console.clear(0, 0, 0, 0);

        // TODO: disable the cursor

        // draw the title
        self.set_title_region(console);
        println!("Initium");

        /*

        - disable cursor

        - draw title

        - render help text

        - set new draw region
        - render window based on type
        */
    }
}
