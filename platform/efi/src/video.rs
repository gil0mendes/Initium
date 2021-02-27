use uefi::proto::console::gop::{GraphicsOutput, PixelFormat as EFIPixelFormat};
use uefi::table::boot::BootServices;
use uefi::ResultExt;

use common::{
    console::ConsoleOut,
    video::{FrameBuffer, PixelFormat, VideoManager, VideoMode},
};

/// Converts a PixelFormat from UEFI crate into our common type
fn convert_pixel_format(format: EFIPixelFormat) -> PixelFormat {
    match format {
        EFIPixelFormat::Rgb => PixelFormat::Rgb,
        EFIPixelFormat::Bgr => PixelFormat::Bgr,
        EFIPixelFormat::Bitmask => PixelFormat::Bitmask,
        _ => PixelFormat::BltOnly,
    }
}

static mut GRAPHICS_OUTPUT: Option<&mut GraphicsOutput<'static>> = None;

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
            .map(|mode| mode.expect("Warnings encountered while querying mode"))
            .find(|ref mode| {
                let info = mode.info();

                info.resolution() == (1024, 768)
            })
            .unwrap();

        gop.set_mode(&mode)
            .expect_success("Failed to set graphics mode.");
    }

    pub fn init(bt: &BootServices) {
        // Look for a graphics output handler
        let mut gop_proto = bt
            .locate_protocol::<GraphicsOutput>()
            .expect_success("UEFI Graphics Output Protocol is not supported");
        let gop = unsafe { &mut *gop_proto.get() };

        unsafe {
            GRAPHICS_OUTPUT = Some(gop);
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
