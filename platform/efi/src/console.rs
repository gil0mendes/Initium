use common::video::{ConsoleOut, FrameBuffer, VideoManager, PixelFormat, VideoMode};
use common::console::{DrawRegion, Char, FONT_WIDTH, FONT_HEIGHT};
use crate::video::{EFIVideoManager, VIDEO_MANAGER};
use crate::platform_manager;
use common::PlatformManager;
use alloc::boxed::Box;
use alloc::vec;
use alloc::vec::Vec;
use rlibc::memset;

/// Type for the cursor position
type CursorPos = (usize, usize);


/// Get the byte offset of a pixel.
#[inline]
fn fb_offset(x: u32, y: u32, stride: u32) -> usize {
    (((y * stride) + x) * 4) as usize
}

pub static mut CONSOLE_MANAGER: Option<ConsoleOutManager> = None;

pub struct ConsoleOutManager {
    /// number of columns on the console
    cols: usize,
    /// number of rows on the console
    rows: usize,

    /// Current draw region
    region: DrawRegion,
    /// Cursor position
    cursor_pos: CursorPos,
    /// Console chars
    chars: Box<Vec<Char>>,
}

impl ConsoleOutManager {
    /// Create a new console out manager instance
    pub fn init() {
        let default_draw_region = DrawRegion {
            x: 0,
            y: 0,
            width: 0,
            height: 0,
            scrollable: false,
        };

        let mut manager = Self {
            cols: 0,
            rows: 0,
            region: default_draw_region,
            cursor_pos: (0, 0),
            chars: Box::new(Vec::new()),
        };

        unsafe { CONSOLE_MANAGER = Some(manager); };
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

    /// Get current video mode
    fn get_current_mode(&self) -> VideoMode {
        unsafe {
            use common::PlatformManager;

            let platform = platform_manager();
            let video_manager = platform.as_ref().video_manager();
            video_manager.as_ref().get_mode()
        }
    }

    /// Draw a rectangle in a solid color.
    fn fillrect(&self, x: u32, y: u32, width: usize, height: usize, rbg: u32) {
        let current_mode = self.get_current_mode();

        if x == 0 && width == current_mode.width && (rbg == 0 || rbg == 0xffffff) {
            unsafe {
                let mut video_manager = VIDEO_MANAGER.unwrap();
                let mut framebuffer = video_manager.get_framebuffer();

                let offset = fb_offset(x, y, current_mode.stride as u32);
                let base_addr = framebuffer.as_mut_ptr().add(offset);
                let size = (height * current_mode.stride) * 4;
                memset(base_addr, rbg as i32, size);
            }
        } else {
            // TODO: implement a pixel by pixel approach for the other cases
            unimplemented!()
        }
    }
}

impl ConsoleOut for ConsoleOutManager {
    fn init(&mut self) {
        let current_mode = self.get_current_mode();

        self.cols = current_mode.width / FONT_WIDTH;
        self.rows = current_mode.height / FONT_HEIGHT;

        // initialize chars array and zero the vector
        let size = self.cols * self.rows;
        self.chars = Box::new(vec![Char::default(); size as usize]);

        self.fillrect(0, 0, current_mode.width, current_mode.height, 0xffffff);

        self.region = DrawRegion {
            x: 0,
            y: 0,
            width: current_mode.width as u16,
            height: current_mode.height as u16,
            scrollable: false,
        }
    }

    fn clear(&mut self, x: u16, y: u16, mut width: u16, mut height: u16) {
        assert!(x + width <= self.region.width);
        assert!(y + height <= self.region.height);

        let stride = unsafe {
            platform_manager()
                .as_ref()
                .video_manager()
                .as_ref()
                .get_mode()
                .stride
        };

        if width == 0 {
            width = self.region.width - x;
        }
        if height == 0 {
            height = self.region.height - y;
        }

        for row in 0..height {
            for col in 0..width {
                // TODO: remove old code
                let offset = fb_offset((x + col) as u32, (y + row) as u32, stride as u32);
                self.write_pixel(offset, [(row % 255) as u8, (col % 255) as u8, (row % 255) as u8]);
            }
        }
    }
}
