use core::fmt::Write;

use crate::key::Key;

/// Console colors
#[derive(Clone, Copy)]
pub enum Color {
    Black = 0x000000,
    Blue = 0x0000aa,
    Green = 0x00aa00,
    Cyan = 0x00aaaa,
    Red = 0xaa0000,
    Magenta = 0xaa00aa,
    Brown = 0xaa5500,
    LightGrey = 0xaaaaaa,
    Grey = 0x555555,
    LightBlue = 0x5555ff,
    LightGreen = 0x55ff55,
    LightCyan = 0x55ffff,
    LightRed = 0xff5555,
    LightMagenta = 0xff55ff,
    Yellow = 0xffff55,
    White = 0xffffff,
}

/// Type for the cursor position
#[derive(Clone, Copy, Debug)]
pub struct Cursor {
    pub x: usize,
    pub y: usize,
    pub visible: bool,
}

impl Cursor {
    pub fn new(x: usize, y: usize, visible: bool) -> Self {
        Cursor { x, y, visible }
    }

    pub fn position(&self) -> (usize, usize) {
        (self.x, self.y)
    }

    pub fn set_position(&mut self, position: (usize, usize)) {
        self.x = position.0;
        self.y = position.1;
    }
}

impl Default for Cursor {
    fn default() -> Self {
        Self {
            x: 0,
            y: 0,
            visible: false,
        }
    }
}

/// Console draw region structure
#[derive(Copy, Clone, Debug)]
pub struct DrawRegion {
    /// X position
    pub x: usize,
    /// Y position
    pub y: usize,
    /// Width of region
    pub width: usize,
    /// Height of region
    pub height: usize,
    /// Whether to scroll when cursor reaches the end
    pub scrollable: bool,
}

pub trait ConsoleOut: Write {
    /// Initialize the console when it is made active.
    fn init(&mut self) {}

    /// Deinitialize the console when it is being made inactive.
    fn deinit(&mut self) {}

    /// Write a character to the console.
    fn put_char(&mut self, ch: char);

    /// Set current colors.
    fn set_color(&mut self, _fg: Color, _bg: Color) {}

    /// Being UI mode
    fn begin_ui(&mut self) {}

    /// End UI mode
    fn end_ui(&mut self);

    /// Set the draw region of the console
    ///
    /// Set the draw region of the console. All operations on the console (i.e. writing, scrolling) will be contained to
    /// this region. The cursor will be moved to 0,0 within this region.
    fn set_region(&mut self, region: DrawRegion);

    /// Reset region for the original console dimensions.
    fn reset_region(&mut self);

    /// Get the current draw region.
    fn get_region(&self) -> DrawRegion;

    /// Configure the cursor
    ///
    /// The position is relative to current active region. This means, for example, that a (0, 0) position can not means
    /// that the cursor will be placed on the corner of the screen, if the region is placed elsewhere.
    fn set_cursor(&mut self, cursor: Cursor);

    /// Get the current cursor state.
    fn get_cursor(&self) -> Cursor;

    /// Clear an area to the current background color.
    ///
    /// # Arguments
    ///
    /// * `x` - Start X position (relative to draw region).
    /// * `y` - Start Y position (relative to draw region).
    /// * ` width` - Width of the area (if 0, whole width is cleared).
    /// * ` height` - Height of the area (if 0, whole height is cleared).
    ///
    fn clear(&mut self, x: usize, y: usize, width: usize, height: usize);

    /// Scroll the draw region up (move contents down)
    fn scroll_up(&mut self);

    /// Scroll the draw region down (move contents up)
    fn scroll_down(&mut self);

    /// Get the console resolution.
    fn resolution(&self) -> (usize, usize);
}

pub trait ConsoleIn {
    /// Check for a character from a console.
    ///
    /// Returns whether a character is available.
    fn poll(&mut self) -> bool;

    /// Read a character from the console.
    fn get_char(&mut self) -> Key;
}
