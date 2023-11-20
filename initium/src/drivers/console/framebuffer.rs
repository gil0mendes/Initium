//! Framebuffer console implementation

use core::fmt;

use alloc::{
    boxed::Box,
    vec::{self, Vec},
};
use common::{
    console::{Color, ConsoleOut, Cursor, DrawRegion},
    video::{FrameBuffer, PixelFormat},
};
use log::info;
use rlibc::memset;

use crate::video::get_video_manager;

use super::font::CONSOLE_FONT;

/// font width
pub const FONT_WIDTH: usize = 8;
/// font height
pub const FONT_HEIGHT: usize = 16;

const COLOR_FG: Color = Color::LightGrey;
const COLOR_BG: Color = Color::Black;

/// Framebuffer character information
#[derive(Copy, Clone)]
pub struct Char {
    /// Character to display (0 == space)
    pub char: char,
    /// Foreground color
    pub foreground: Color,
    /// Background color
    pub background: Color,
}

impl Default for Char {
    fn default() -> Self {
        Self {
            char: ' ',
            foreground: Color::White,
            background: Color::Black,
        }
    }
}

/// Get the byte offset of a pixel.
#[inline]
fn fb_offset(x: usize, y: usize) -> usize {
    let mode = get_video_manager().get_mode_info().unwrap();

    let stride = mode.stride;
    ((y * stride) + x) * 4
}

/// Convert a hexadecimal color into a rgb vector
#[inline]
fn hex_to_rgb(color: u32) -> [u8; 3] {
    [
        (color >> 16) as u8 & 0xFF,
        (color >> 8) as u8 & 0xFF,
        (color as u8 & 0xFF),
    ]
}

pub struct FramebufferConsole<'a> {
    framebuffer: FrameBuffer<'a>,

    /// number of columns on the console
    cols: usize,
    /// number of rows on the console
    rows: usize,

    /// Current draw region
    region: DrawRegion,
    /// Cursor
    cursor: Cursor,
    /// Console chars
    chars: Box<Vec<Char>>,

    /// Foreground color
    foreground_color: Color,
    /// Background color
    background_color: Color,
}

impl<'a> FramebufferConsole<'a> {
    /// Create a new console out manager instance
    pub fn new(width: usize, height: usize, framebuffer: FrameBuffer<'a>) -> Self {
        // Compute the number of rows and cols based ont the resolution and font size
        let cols = width / FONT_WIDTH;
        let rows = height / FONT_HEIGHT;

        let default_draw_region = DrawRegion {
            x: 0,
            y: 0,
            width: cols,
            height,
            scrollable: false,
        };

        // initialize chars array and zero the vector
        let size = cols * rows;
        let chars = Box::new(alloc::vec![Char::default(); size as usize]);

        Self {
            framebuffer,
            cols,
            rows,
            region: default_draw_region,
            cursor: Cursor::default(),
            chars,
            foreground_color: Color::White,
            background_color: Color::Black,
        }
    }

    /// Put a pixel on the framebuffer.
    fn put_pixel(&mut self, x: usize, y: usize, color: u32) {
        let pixel_base = fb_offset(x, y);

        let current_mode = get_video_manager().get_mode_info().unwrap();
        let pixel_format = current_mode.format;

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
            PixelFormat::Rgb => write_pixel_rgb,
            PixelFormat::Bgr => write_pixel_bgr,
            _ => panic!("This pixel format is not support by the drawer"),
        };

        unsafe {
            func(&mut self.framebuffer, pixel_base, rgb);
        }
    }

    /// Draw a rectangle in a solid color.
    fn fillrect(&mut self, x: u32, y: u32, width: usize, height: usize, rbg: u32) {
        let current_mode = get_video_manager().get_mode_info().unwrap();

        if x == 0 && width == current_mode.width && (rbg == 0 || rbg == 0xffffff) {
            unsafe {
                let offset = fb_offset(x as usize, y as usize);
                let base_addr = self.framebuffer.as_mut_ptr().add(offset);
                let size = (height * current_mode.stride) * 4;
                memset(base_addr, rbg as i32, size);
            }
        } else {
            for i in 0..=height {
                for j in 0..=width {
                    self.put_pixel(x as usize + j as usize, y as usize + i, rbg);
                }
            }
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
                    self.put_pixel(x + col, y + row, ch.foreground as u32);
                } else {
                    self.put_pixel(x + col, y + row, ch.background as u32);
                }
            }
        }
    }

    /// Toggle the cursor if enabled.
    fn toggle_cursor(&mut self) {
        if !self.cursor.visible {
            return;
        }

        let idx = (self.cursor.y * self.cols) + self.cursor.x;

        if self.chars[idx].char as u8 > 0 {
            // invert the colors
            let temp = self.chars[idx].foreground;
            self.chars[idx].foreground = self.chars[idx].background;
            self.chars[idx].background = temp;
        } else {
            // nothing has yet been written, initialize the character. we must be enabling the cursor if this is the
            // case, so invert colors.
            self.chars[idx].char = ' ';
            self.chars[idx].foreground = Color::Black;
            self.chars[idx].background = Color::LightGrey;
        }

        // redraw glyph with the new colors
        self.draw_glyph(self.cursor.x, self.cursor.y);
    }
}

impl<'a> ConsoleOut for FramebufferConsole<'a> {
    fn put_char(&mut self, ch: char) {
        self.toggle_cursor();

        match ch {
            // backspace, move back one character of we can
            '\x08' => {
                if self.cursor.x > self.region.x {
                    self.cursor.x = self.cursor.x - 1;
                } else if self.cursor.y > self.region.y {
                    self.cursor
                        .set_position((self.region.x + self.region.width - 1, self.cursor.y - 1));
                }
            }
            // carriage return, move to the start of the line
            '\r' => {
                self.cursor.x = self.region.x;
            }
            // newline, treat it as if a carriage return was there
            '\n' => {
                self.cursor.set_position((self.region.x, self.cursor.y + 1));
            }
            // tab
            '\t' => {
                self.cursor.x = self.cursor.x + 8 - (self.cursor.x % 8);
            }
            // only deal with printable chars
            _ => {
                if ch >= ' ' {
                    let idx = (self.cursor.y * self.cols) + self.cursor.x;
                    self.chars[idx].char = ch;
                    self.chars[idx].foreground = self.foreground_color;
                    self.chars[idx].background = self.background_color;

                    self.draw_glyph(self.cursor.x, self.cursor.y);

                    self.cursor.x = self.cursor.x + 1;
                }
            }
        }

        let (x, y) = self.cursor.position();

        // if we have reached the edge of the draw region insert a new line
        if x >= self.region.x + self.region.width {
            self.cursor.set_position((self.region.x, y + 1));
        }

        // if we have reached the bottom of the draw region, scroll
        if y > self.region.y + self.region.height {
            if self.region.scrollable {
                self.scroll_down();
            }

            // update the cursor position
            self.cursor.y = self.region.y + self.region.height - 1;
        }

        self.toggle_cursor();
    }

    fn end_ui(&mut self) {
        todo!()
    }

    fn set_region(&mut self, region: DrawRegion) {
        assert!(region.width > 0 && region.height > 0);
        assert!(region.x + region.width <= self.cols);
        assert!(region.y + region.height <= self.rows);

        self.region = region;

        // adjust cursor position
        self.set_cursor(Cursor::new(0, 0, self.cursor.visible));
    }

    fn reset_region(&mut self) {
        self.region.x = 0;
        self.region.y = 0;
        self.region.width = self.cols;
        self.region.height = self.rows;
        self.region.scrollable = true;
    }

    fn get_region(&self) -> DrawRegion {
        self.region.clone()
    }

    fn set_cursor(&mut self, cursor: Cursor) {
        assert!(cursor.x < self.region.width);
        assert!(cursor.y < self.region.height);

        self.toggle_cursor();
        self.cursor
            .set_position((self.region.x + cursor.x, self.region.y + cursor.y));
        self.cursor.visible = cursor.visible;
        self.toggle_cursor();
    }

    fn get_cursor(&self) -> Cursor {
        self.cursor
    }

    fn clear(&mut self, x: usize, y: usize, mut width: usize, mut height: usize) {
        assert!(x + width <= self.region.width);
        assert!(y + height <= self.region.height);

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

                if self.cursor.visible && abs_x == self.cursor.x && abs_y == self.cursor.y {
                    // avoid redrawing the glyph twice when cursor is active
                    self.toggle_cursor();
                } else {
                    self.draw_glyph(abs_x, abs_y);
                }
            }
        }
    }

    fn scroll_up(&mut self) {
        todo!()
    }

    fn scroll_down(&mut self) {
        todo!()
    }

    fn resolution(&self) -> (usize, usize) {
        (self.cols, self.rows)
    }

    fn set_color(&mut self, fg: Color, bg: Color) {
        self.foreground_color = fg;
        self.background_color = bg;
    }

    fn init(&mut self) {
        // TODO: add video type validation. For that, we need to stope the type on VideoMode

        let current_mode = get_video_manager().get_mode_info().unwrap();

        // clear console
        self.fillrect(
            0,
            0,
            current_mode.width,
            current_mode.height,
            COLOR_BG as u32,
        );
        self.toggle_cursor();
    }
}

impl<'a> fmt::Write for FramebufferConsole<'a> {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        s.chars().for_each(|c| self.put_char(c));

        Ok(())
    }
}
