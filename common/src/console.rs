/// Default console background color
pub const DEFAULT_BG: u32 = 0x000000;

/// Default console foreground color
pub const DEFAULT_FG: u32 = 0xFFFFFF;

/// font width
pub const FONT_WIDTH: usize = 8;
/// font height
pub const FONT_HEIGHT: usize = 16;

/// Console draw region structure
#[derive(Copy, Clone)]
pub struct DrawRegion {
    /// X position
    pub x: u16,
    /// Y position
    pub y: u16,
    /// Width of region
    pub width: u16,
    /// Height of region
    pub height: u16,
    /// Whether to scroll when cursor reaches the end
    pub scrollable: bool,
}

/// Framebuffer character information
#[derive(Copy, Clone)]
pub struct Char {
    /// Character to display (0 == space)
    pub char: char,
    /// Foreground color
    pub fb: u32,
    /// Background color
    pub bg: u32,
}

impl Default for Char {
    fn default() -> Self {
        Self {
            char: ' ',
            fb: DEFAULT_FG,
            bg: DEFAULT_FG,
        }
    }
}
