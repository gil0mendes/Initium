#![no_std]

use core::ptr::NonNull;

pub mod console;
pub mod font;
pub mod video;

/// Trait to be implemented by the struct to be passed into the generic bootloader code
pub trait PlatformManager {
    /// Get a pointer to video manager
    fn video_manager(&self) -> NonNull<video::VideoManager>;

    /// Get a pointer to console manager
    fn console_manager(&self) -> NonNull<video::ConsoleOut>;
}
