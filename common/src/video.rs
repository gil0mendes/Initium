use core::marker::PhantomData;

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

    /// Get console output
    fn get_console_out(&self) -> &ConsoleOut;
}

/// Framebuffer console state
pub struct FrameBuffer<'gop> {
    /// base address of the framebuffer mapping
    base: *mut u8,
    /// Size of the virtual memory size
    size: usize,
    /// Use a phantom to assign a lifetime to the unsafe pointer
    _lifetime: PhantomData<&'gop mut u8>,
}

impl<'gop> FrameBuffer<'gop> {
    /// Create a new Framebuffer instance
    pub fn new(base: *mut u8, size: usize) -> Self {
        Self {
            base,
            size,
            _lifetime: PhantomData,
        }
    }
}

pub trait ConsoleOut {
    /// Clear an area to the current background color.
    fn clear(&self, x: u16, y: u16, width: u16, height: u16);
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
