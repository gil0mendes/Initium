use crate::key::Key;

/// font width
pub const FONT_WIDTH: usize = 8;
/// font height
pub const FONT_HEIGHT: usize = 16;

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
    Light_grey = 0xaaaaaa,
    Grey = 0x555555,
    Light_blue = 0x5555ff,
    Light_green = 0x55ff55,
    Light_cyan = 0x55ffff,
    Light_red = 0xff5555,
    Light_magenta = 0xff55ff,
    Yellow = 0xffff55,
    White = 0xffffff,
}

/// Console draw region structure
#[derive(Copy, Clone)]
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

/// Framebuffer character information
#[derive(Copy, Clone)]
pub struct Char {
    /// Character to display (0 == space)
    pub char: char,
    /// Foreground color
    pub foreground: Color,
    /// Background color
    pub background: Color,
}

impl Default for Char {
    fn default() -> Self {
        Self {
            char: ' ',
            foreground: Color::White,
            background: Color::Black,
        }
    }
}

pub trait ConsoleOut {
    /// Initialize the console when it is made active.
    fn init(&mut self);

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

    /// Set the draw region of the console
    fn set_region(&mut self, region: DrawRegion);

    /// Get the current active region
    fn get_region(&self) -> DrawRegion;

    /// Reset region to the initial values
    fn reset_region(&mut self);

    /// Set current colors.
    fn set_color(&mut self, fg: Color, bg: Color);

    /// Get console resolution
    fn resolution(&self) -> (usize, usize);

    /// Configure the cursor
    ///
    /// The position is relative to current active region. This means, for example, that a (0, 0) position can not means
    /// that the cursor will be placed on the corner of the screen, if the region is placed elsewhere.
    fn set_cursor(&mut self, x: usize, y: usize, visible: bool);
}

pub trait ConsoleIn {
    /// Check for a character from a console.
    ///
    /// Returns whether a character is available.
    fn poll(&mut self) -> bool;

    /// Read a character from the console.
    fn get_char(&mut self) -> Key;
}
