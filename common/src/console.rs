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
