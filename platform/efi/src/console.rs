use common::video::{ConsoleOut, FrameBuffer, VideoManager, PixelFormat};
use common::console::DrawRegion;
use crate::video::{EFIVideoManager, VIDEO_MANAGER};
use crate::platform_manager;
use common::PlatformManager;

// Dimensions of the console font.
const FONT_WIDTH: usize = 8;
const FONT_HEIGHT: usize = 16;

/// Type for the cursor position
type CursorPos = (usize, usize);

pub static mut CONSOLE_MANAGER: Option<ConsoleOutManager> = None;

#[derive(Copy, Clone)]
pub struct ConsoleOutManager {
    /// framebuffer virtual memory map
    // framebuffer: FrameBuffer<'static>,

    /// number of columns on the console
    cols: usize,
    /// number of rows on the console
    rows: usize,

    /// Current draw region
    region: DrawRegion,
    /// Cursor position
    cursor_pos: CursorPos,
}

impl ConsoleOutManager {
    /// Create a new console out manager instance
    pub fn new() -> Self {
        let default_draw_region = DrawRegion {
            x: 0,
            y: 0,
            width: 0,
            height: 0,
            scrollable: false,
        };

        // let framebuffer = unsafe { VIDEO_MANAGER.unwrap().get_framebuffer() };

        let mut manager = Self {
            // framebuffer,
            cols: 0,
            rows: 0,
            region: default_draw_region,
            cursor_pos: (0, 0),
        };

        unsafe { CONSOLE_MANAGER = Some(manager); };

        manager
    }

    fn write_pixel(&mut self, pixel_base: usize, rgb: [u8; 3]) {
        let pixel_format = unsafe {
            use common::PlatformManager;

            let platform = platform_manager();
            let video_manager = platform.as_ref().video_manager();
            video_manager.as_ref().get_mode().format
        };

        type PixelWriter = unsafe fn(&mut FrameBuffer, usize, [u8; 3]);

        /// Write a RGB pixel into the framebuffer
        unsafe fn write_pixel_rgb(framebuffer: &mut FrameBuffer, pixel_base: usize, rgb: [u8; 3]) {
            framebuffer.write_value(pixel_base, rgb)
        }

        /// Write a BGR pixel into the framebuffer
        unsafe fn write_pixel_bgr(framebuffer: &mut FrameBuffer, pixel_base: usize, rgb: [u8; 3]) {
            framebuffer.write_value(pixel_base, [rgb[2], rgb[1], rgb[0]])
        }

        let func: PixelWriter = match pixel_format {
            PixelFormat::RGB => write_pixel_rgb,
            PixelFormat::BGR => write_pixel_bgr,
            _ => panic!("This piel format is not support by the drawing")
        };

        unsafe {
            let mut video_manager = VIDEO_MANAGER.unwrap();
            let mut framebuffer = video_manager.get_framebuffer();
            func(&mut framebuffer, pixel_base, rgb);
        }
    }
}

impl ConsoleOut for ConsoleOutManager {
    fn init(&mut self) {
        let current_mode = unsafe {
            use common::PlatformManager;

            let platform = platform_manager();
            let video_manager = platform.as_ref().video_manager();
            video_manager.as_ref().get_mode()
        };

        self.cols = current_mode.width / FONT_WIDTH;
        self.rows = current_mode.height / FONT_HEIGHT;

        self.region = DrawRegion {
            x: 0,
            y: 0,
            width: current_mode.width as u16,
            height: current_mode.height as u16,
            scrollable: false,
        }
    }

    fn clear(&mut self, x: u16, y: u16, mut width: u16, mut height: u16) {
        let stride = unsafe {
            platform_manager()
                .as_ref()
                .video_manager()
                .as_ref()
                .get_mode()
                .stride
        };

        width = self.region.width - x;
        height = self.region.height - y;

        for row in 0..height {
            for col in 0..width {
                let pixel_index = (row as usize * stride) + col as usize;
                let pixel_base = 4 * pixel_index;
                self.write_pixel(pixel_base as usize, [200, 200, 200]);
            }
        }
    }
}
