use alloc::boxed::Box;
use common::video::{VideoModeInfo, VideoModeOps};
use uefi::proto::console::gop::{GraphicsOutput, Mode, PixelFormat as EFIPixelFormat};
use uefi::table::boot::{BootServices, ScopedProtocol};

use common::{
    console::ConsoleOut,
    video::{FrameBuffer, PixelFormat},
};

use crate::drivers::console::framebuffer::FramebufferConsole;
use crate::video::get_video_manager;

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
pub static mut EFI_VIDEO_MANAGER: Option<EFIVideoManager> = None;

struct EfiVideoMode {
    mode: Mode,
    video_mode: VideoModeInfo,
}

impl VideoModeOps for EfiVideoMode {
    fn set_mode(&mut self) {
        let gop = unsafe { GRAPHICS_OUTPUT.as_mut().unwrap() };

        gop.set_mode(&self.mode);

        // TODO: set the mem_phys, mem_virt, mem_size
    }

    fn create_console(&self) -> Option<Box<dyn ConsoleOut>> {
        let gop = unsafe { GRAPHICS_OUTPUT.as_mut().unwrap() };
        let mut platform_framebuffer = gop.frame_buffer();
        let framebuffer = FrameBuffer::new(
            platform_framebuffer.as_mut_ptr(),
            platform_framebuffer.size(),
        );

        Some(Box::new(FramebufferConsole::new(
            self.video_mode.width,
            self.video_mode.height,
            framebuffer,
        )))
    }

    fn get_mode_info(&self) -> VideoModeInfo {
        self.video_mode
    }
}

/// Manager for EFI video related opetations
pub struct EFIVideoManager {
    /// Original video mode
    ///
    /// We use this to return to if we exit Initium.
    original_mode: Mode,
}

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
    fn set_init_graphics_mode(&mut self, bt: &BootServices) {
        let mut gop = unsafe {
            match GRAPHICS_OUTPUT {
                Some(ref mut gop) => gop,
                _ => panic!("A reference for the Graphics Output protocol is required"),
            }
        };

        let mode = gop
            .modes(bt)
            .find(|ref mode| {
                let info = mode.info();

                info.resolution() == (1024, 768)
            })
            .unwrap();

        gop.set_mode(&mode).expect("Failed to set graphics mode.");

        let mode_info = mode.info();
        let (width, height) = mode_info.resolution();
        let video_mode_info = VideoModeInfo {
            width,
            height,
            stride: mode_info.stride(),
            format: convert_pixel_format(mode_info.pixel_format()),
        };

        let mut video_mode = EfiVideoMode {
            mode,
            video_mode: video_mode_info,
        };

        get_video_manager().set_mode(Some(Box::new(video_mode)), true);
    }

    pub fn init() {
        let bs = unsafe { boot_services().as_ref() };

        // Look for a graphics output handler
        let handle = bs
            .get_handle_for_protocol::<GraphicsOutput>()
            .expect("efi: Graphics Output Protocol is not supported");

        let mut gop_proto = bs
            .open_protocol_exclusive::<GraphicsOutput>(handle)
            .expect("efi: Graphics Output Protocol is not supported");

        let current_mode = gop_proto.current_mode_info();
        let original_mode = gop_proto
            .modes(bs)
            .find(|m| current_mode == *m.info())
            .expect("efi: impossible to get the current video mode");

        unsafe {
            GRAPHICS_OUTPUT = Some(gop_proto);
        };

        let mut manager = Self {
            original_mode: original_mode,
        };
        manager.set_init_graphics_mode(&bs);

        unsafe {
            EFI_VIDEO_MANAGER = Some(manager);
        }
    }
}

// impl VideoManager for EFIVideoManager {
//     fn get_mode(&self) -> VideoMode {
//         let graphics = unsafe {
//             match GRAPHICS_OUTPUT {
//                 Some(ref mut gop) => gop,
//                 _ => panic!(),
//             }
//         };

//         let current_mode = graphics.current_mode_info();
//         let resolution = current_mode.resolution();

//         VideoMode {
//             width: resolution.0,
//             height: resolution.1,
//             format: convert_pixel_format(current_mode.pixel_format()),
//             stride: current_mode.stride(),
//         }
//     }

//     fn get_console_out(&self) -> &dyn ConsoleOut {
//         todo!()
//     }
// }
