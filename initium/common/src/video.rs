//! Video module
//!
//! This files defines all the necessary structures to manage video and graphics inside Initium

use core::marker::PhantomData;
use core::mem;

use alloc::boxed::Box;

use crate::console::ConsoleOut;

/// Represents the format of the pixels in a frame buffer.
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(u32)]
pub enum PixelFormat {
    /// Each pixel is 32-bit long, with 24-bit RGB, and the last byte is reserved.
    Rgb = 0,
    /// Each pixel is 32-bit long, with 24-bit BGR, and the last byte is reserved.
    Bgr,
    /// Custom pixel format, check the associated bitmask.
    Bitmask,
    /// The graphics mode does not support drawing directly to the frame buffer.
    ///
    /// This means you will have to use the `blt` function which will convert the graphics data to the device's internal
    /// pixel format.
    BltOnly,
    // SAFETY: UEFI also defines a PixelFormatMax variant, and states that all
    //         valid enum values are guaranteed to be smaller. Since that is the
    //         case, adding a new enum variant would be a breaking change, so it
    //         is safe to model this C enum as a Rust enum.
}

/// Video mode types
pub enum VideoModeType {
    VGA = (1 << 0),
    LFB = (1 << 1),
}

/// Video mode information
#[derive(Clone, Copy)]
pub struct VideoMode {
    /// Number of pixels on the width
    pub width: usize,
    /// Number os pixels on the height
    pub height: usize,
    /// Pixel format
    // pub format: PixelFormat,
    /// Number of pixels per scanline
    pub stride: usize,
}

/// Operations that a mode needs to implement
pub trait VideoModeOps {
    /// Set the mode.
    fn set_mode();

    /// Create a console for the mode.
    fn create_console() -> Option<Box<dyn ConsoleOut>> {
        None
    }
}

/// Trait that must be implemented by the platform video manager.
///
/// This is the way that we have to enforce the use of the same structure across platforms. The code on the platform can
/// change of the the implementation on the main bootloader code can remain abstract.
pub trait VideoManager {
    /// Get console output
    fn get_console_out(&self) -> &dyn ConsoleOut;
}

/// Framebuffer console state
#[derive(Copy, Clone)]
pub struct FrameBuffer<'a> {
    /// base address of the framebuffer mapping
    base: *mut u8,
    /// Size of the virtual memory size
    size: usize,
    /// Use a phantom to assign a lifetime to the unsafe pointer
    _lifetime: PhantomData<&'a mut u8>,
}

impl<'a> FrameBuffer<'a> {
    /// Create a new Framebuffer instance
    pub fn new(base: *mut u8, size: usize) -> Self {
        Self {
            base,
            size,
            _lifetime: PhantomData,
        }
    }

    /// Access the raw framebuffer pointer
    ///
    /// To use this pointer safely and correctly, you must...
    /// - honor the pixel format and stride specified by the mode info
    /// - keep memory accesses in bound
    /// - use volatile reads and writes
    /// - make sure that the pointer does not outlive the FrameBuffer
    pub fn as_mut_ptr(&mut self) -> *mut u8 {
        self.base
    }

    /// Query the framebuffer size in bytes
    pub fn size(&self) -> usize {
        self.size
    }

    /// Modify the i-th byte of the frame buffer
    ///
    /// # Safety
    ///
    /// This operation is unsafe because...
    /// - you must honor the pixel format and stride specified by the mode info
    /// - there is no bound checking on memory accesses in release mode
    #[inline]
    pub unsafe fn write_byte(&mut self, index: usize, value: u8) {
        debug_assert!(index < self.size, "Frame buffer accessed out of bounds");
        self.base.add(index).write_volatile(value)
    }

    /// Read the i-th byte of the frame buffer
    ///
    /// # Safety
    ///
    /// This operation is unsafe because...
    /// - you must honor the pixel format and stride specified by the mode info
    /// - there is no bound checking on memory accesses in release mode
    #[inline]
    pub unsafe fn read_byte(&self, index: usize) -> u8 {
        debug_assert!(index < self.size, "Frame buffer accessed out of bounds");
        self.base.add(index).read_volatile()
    }

    #[inline]
    pub unsafe fn write_value<T>(&mut self, index: usize, value: T) {
        debug_assert!(
            index.saturating_add(mem::size_of::<T>()) <= self.size,
            "Frame buffer accessed out of bounds"
        );
        (self.base.add(index) as *mut T).write_volatile(value)
    }
}
