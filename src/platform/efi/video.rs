use uefi::proto::console::gop::{GraphicsOutput, PixelFormat as EFIPixelFormat};
use uefi::table::boot::{BootServices, ScopedProtocol};

use common::{
    console::ConsoleOut,
    video::{FrameBuffer, PixelFormat, VideoManager, VideoMode},
};

use super::allocator::boot_services;

/// Converts a PixelFormat from UEFI crate into our common type
fn convert_pixel_format(format: EFIPixelFormat) -> PixelFormat {
    match format {
        EFIPixelFormat::Rgb => PixelFormat::Rgb,
        EFIPixelFormat::Bgr => PixelFormat::Bgr,
        EFIPixelFormat::Bitmask => PixelFormat::Bitmask,
        _ => PixelFormat::BltOnly,
    }
}

static mut GRAPHICS_OUTPUT: Option<ScopedProtocol<GraphicsOutput>> = None;

/// Reference for the EFI video manager
pub static mut VIDEO_MANAGER: Option<EFIVideoManager> = None;

/// Manager for EFI video related opetations
#[derive(Copy, Clone)]
pub struct EFIVideoManager;

impl EFIVideoManager {
    /// Get the framebuffer
    pub fn get_framebuffer(&mut self) -> FrameBuffer {
        unsafe {
            match GRAPHICS_OUTPUT {
                Some(ref mut gop) => {
                    let mut platform_framebuffer = gop.frame_buffer();
                    return FrameBuffer::new(
                        platform_framebuffer.as_mut_ptr(),
                        platform_framebuffer.size(),
                    );
                }
                _ => panic!(),
            }
        };
    }

    /// Set a large graphics mode.
    fn set_init_graphics_mode(&mut self) {
        let mut gop = unsafe {
            match GRAPHICS_OUTPUT {
                Some(ref mut gop) => gop,
                _ => panic!("A reference for the Graphics Output protocol is required"),
            }
        };

        let mode = gop
            .modes()
            .find(|ref mode| {
                let info = mode.info();

                info.resolution() == (1024, 768)
            })
            .unwrap();

        gop.set_mode(&mode).expect("Failed to set graphics mode.");
    }

    pub fn init() {
        let bt = unsafe { boot_services().as_ref() };

        // Look for a graphics output handler
        let handle = bt
            .get_handle_for_protocol::<GraphicsOutput>()
            .expect("efi: Graphics Output Protocol is not supported");

        let mut gop_proto = bt
            .open_protocol_exclusive::<GraphicsOutput>(handle)
            .expect("efi: Graphics Output Protocol is not supported");

        unsafe {
            GRAPHICS_OUTPUT = Some(gop_proto);
        };

        let mut manager = Self {};
        manager.set_init_graphics_mode();

        unsafe {
            VIDEO_MANAGER = Some(manager);
        }
    }
}

impl VideoManager for EFIVideoManager {
    fn get_mode(&self) -> VideoMode {
        let graphics = unsafe {
            match GRAPHICS_OUTPUT {
                Some(ref mut gop) => gop,
                _ => panic!(),
            }
        };

        let current_mode = graphics.current_mode_info();
        let resolution = current_mode.resolution();

        VideoMode {
            width: resolution.0,
            height: resolution.1,
            format: convert_pixel_format(current_mode.pixel_format()),
            stride: current_mode.stride(),
        }
    }

    fn get_console_out(&self) -> &dyn ConsoleOut {
        todo!()
    }
}
