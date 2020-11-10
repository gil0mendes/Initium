///! Video module
///!
///! This files defines all the necessary structures to manage video and graphics inside Initium

/// Video mode information
pub struct VideoMode {
    /// Number of pixels on the width
    pub width: u32,
    /// Number os pixels on the height
    pub height: u32,
}

/// Trait that must be implemented by the platform video manager.
///
/// This is the way that we have to enforce the use of the same structure across platforms. The code on the platform can
/// change of the the implementation on the main bootloader code can remain abstract.
pub trait VideoManager {
    /// Get the current video mode set
    fn get_mode(&self) -> VideoMode;
}


/*
pub struct ModeInfo {
    // The only known version, associated with the current spec, is 0.
    version: u32,
    hor_res: u32,
    ver_res: u32,
    format: PixelFormat,
    mask: PixelBitmask,
    stride: u32,
}
*/
