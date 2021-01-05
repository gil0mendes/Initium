use crate::video::VIDEO_MANAGER;
use alloc::boxed::Box;
use alloc::vec;
use alloc::vec::Vec;
use common::video::{ConsoleOut, FrameBuffer, PixelFormat, VideoMode};
use common::{
    console::{Char, Color, DrawRegion, FONT_HEIGHT, FONT_WIDTH},
    font::CONSOLE_FONT,
};
use core::fmt;
use core::result::Result;
use rlibc::memset;

/// Print with new line to console
#[macro_export]
macro_rules! println {
    ($fmt:expr) => (print!(concat!($fmt, "\n")));
    ($fmt:expr, $($arg:tt)*) => (print!(concat!($fmt, "\n"), $($arg)*));
}

/// Print to console
#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ({
        crate::platform::console::print(format_args!($($arg)*)).unwrap();
    });
}

pub fn print(args: fmt::Arguments) -> fmt::Result {
    use core::fmt::Write;
    unsafe {
        let mut console = &mut CONSOLE_MANAGER;
        console.as_mut().unwrap().write_fmt(args)
    }
}

/// Type for the cursor position
type CursorPos = (usize, usize);

pub static mut CONSOLE_MANAGER: Option<ConsoleOutManager> = None;

/// Convert a hexadecimal color into a rgb vector
fn hex_to_rgb(color: u32) -> [u8; 3] {
    [
        (color >> 16) as u8 & 0xFF,
        (color >> 8) as u8 & 0xFF,
        (color as u8 & 0xFF),
    ]
}

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

    /// Foreground color
    foreground_color: Color,
    /// Background color
    background_color: Color,
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
            foreground_color: Color::White,
            background_color: Color::Black,
        };

        unsafe {
            CONSOLE_MANAGER = Some(manager);
        };
    }

    /// Get the byte offset of a pixel.
    #[inline]
    fn fb_offset(&self, x: usize, y: usize) -> usize {
        use common::video::VideoManager;
        let stride = unsafe { VIDEO_MANAGER.unwrap().get_mode().stride };
        (((y * stride) + x) * 4)
    }

    fn write_pixel(&mut self, x: usize, y: usize, color: u32) {
        let pixel_base = self.fb_offset(x, y);
        let pixel_format = unsafe {
            use common::video::VideoManager;
            VIDEO_MANAGER.unwrap().get_mode().format
        };

        // convert rgb url into a rgb vector
        let rgb: [u8; 3] = hex_to_rgb(color);

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
            _ => panic!("This pixel format is not support by the drawing"),
        };

        unsafe {
            let mut video_manager = VIDEO_MANAGER.unwrap();
            let mut framebuffer = video_manager.get_framebuffer();
            func(&mut framebuffer, pixel_base, rgb);
        }
    }

    fn putc(&mut self, ch: char) {
        let (x, y) = self.cursor_pos;

        let abs_x = self.region.x + x;
        let abs_y = self.region.y + y;
        let idx = (abs_y * self.cols) + abs_x;

        self.chars[idx].char = ch;
        self.chars[idx].foreground = self.foreground_color;
        self.chars[idx].background = self.background_color;

        self.draw_glyph(abs_x, abs_y);

        // TODO: implement scroll
        if x + 1 < self.cols {
            self.cursor_pos = (x + 1, y);
        } else {
            self.cursor_pos = (0, y + 1);
        }
    }

    /// Get current video mode
    fn get_current_mode(&self) -> VideoMode {
        use common::video::VideoManager;
        unsafe { VIDEO_MANAGER.unwrap().get_mode() }
    }

    /// Draw a rectangle in a solid color.
    fn fillrect(&self, x: u32, y: u32, width: usize, height: usize, rbg: u32) {
        let current_mode = self.get_current_mode();

        if x == 0 && width == current_mode.width && (rbg == 0 || rbg == 0xffffff) {
            unsafe {
                let mut video_manager = VIDEO_MANAGER.unwrap();
                let mut framebuffer = video_manager.get_framebuffer();

                let offset = self.fb_offset(x as usize, y as usize);
                let base_addr = framebuffer.as_mut_ptr().add(offset);
                let size = (height * current_mode.stride) * 4;
                memset(base_addr, rbg as i32, size);
            }
        } else {
            // TODO: implement a pixel by pixel approach for the other cases
            unimplemented!()
        }
    }

    /// Draw the glyph at the specified position of the console.
    fn draw_glyph(&mut self, x: usize, y: usize) {
        let idx = (y * self.cols) + x;
        let ch = self.chars[idx];

        // convert into a pixel positions
        let x = x * FONT_WIDTH;
        let y = y * FONT_HEIGHT;

        for row in 0..FONT_HEIGHT {
            for col in 0..FONT_WIDTH {
                let char_index = (ch.char as usize * FONT_HEIGHT) + row;
                let font_char = CONSOLE_FONT[char_index];

                if (font_char & (1 << (7 - col))) > 0 {
                    self.write_pixel(x + col, y + row, ch.foreground as u32);
                } else {
                    self.write_pixel(x + col, y + row, ch.background as u32);
                }
            }
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
            width: current_mode.width,
            height: current_mode.height,
            scrollable: false,
        }
    }

    fn clear(&mut self, x: usize, y: usize, mut width: usize, mut height: usize) {
        use common::video::VideoManager;

        assert!(x + width <= self.region.width);
        assert!(y + height <= self.region.height);

        let stride = unsafe { VIDEO_MANAGER.unwrap().get_mode().stride };

        if width == 0 {
            width = self.region.width - x;
        }
        if height == 0 {
            height = self.region.height - y;
        }

        for row in 0..height {
            for col in 0..width {
                let abs_x = self.region.x + col;
                let abs_y = self.region.y + row;
                let idx = (abs_y * self.cols) + abs_x;

                self.chars[idx].char = ' ';
                self.chars[idx].foreground = self.foreground_color;
                self.chars[idx].background = self.background_color;

                // TODO: avoid redrawing the glyph twice when cursor is active
                self.draw_glyph(abs_x, abs_y);
            }
        }
    }

    fn set_region(&mut self, region: DrawRegion) {
        assert!(region.width > 0 && region.height > 0);
        assert!(region.x + region.width <= self.cols);
        assert!(region.y + region.height <= self.rows);

        self.region = region;

        // TODO: adjust cursor position
    }

    fn reset_region(&mut self) {
        self.region.x = 0;
        self.region.y = 0;
        self.region.width = self.cols;
        self.region.height = self.rows;
        self.region.scrollable = true;

        // TODO: adjust cursor position
    }

    fn set_color(&mut self, fg: common::console::Color, bg: common::console::Color) {
        self.foreground_color = fg;
        self.background_color = bg;
    }

    fn resolution(&self) -> (usize, usize) {
        (self.cols, self.rows)
    }
}

impl fmt::Write for ConsoleOutManager {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        s.chars().for_each(|c| self.putc(c));

        Result::Ok(())
    }
}
