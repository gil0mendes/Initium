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
