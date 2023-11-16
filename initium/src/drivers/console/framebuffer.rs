//! Framebuffer console implementation

use core::cell::RefCell;

use alloc::{boxed::Box, rc::Rc, vec::Vec};
use common::console::{Color, ConsoleOut, Cursor, DrawRegion};

use crate::video::VIDEO_MANAGER;

use super::font::CONSOLE_FONT;

/// font width
pub const FONT_WIDTH: usize = 8;
/// font height
pub const FONT_HEIGHT: usize = 16;

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
    let stride = unsafe { VIDEO_MANAGER.get_mode().unwrap().stride };
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

pub struct FramebufferConsole {
    /// Console output device
    console: Rc<RefCell<dyn ConsoleOut>>,

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

impl FramebufferConsole {
    /// Create a new console out manager instance
    pub fn new() -> Self {
        let default_draw_region = DrawRegion {
            x: 0,
            y: 0,
            width: 0,
            height: 0,
            scrollable: false,
        };

        Self {
            console: todo!(),
            cols: 0,
            rows: 0,
            region: default_draw_region,
            cursor: Cursor::default(),
            chars: Box::new(Vec::new()),
            foreground_color: Color::White,
            background_color: Color::Black,
        };
    }

    /// Put a pixel on the framebuffer.
    fn put_pixel(&mut self, x: usize, y: usize, color: u32) {
        let pixel_base = fb_offset(x, y);
        unimplemented!("Implement the remaining part");
        // let pixel_format = unsafe { VIDEO_MANAGER.unwrap().get_mode().format };

        // // convert rgb url into a rgb vector
        // let rgb: [u8; 3] = hex_to_rgb(color);

        // type PixelWriter = unsafe fn(&mut FrameBuffer, usize, [u8; 3]);

        // /// Write a RGB pixel into the framebuffer
        // unsafe fn write_pixel_rgb(framebuffer: &mut FrameBuffer, pixel_base: usize, rgb: [u8; 3]) {
        //     framebuffer.write_value(pixel_base, rgb)
        // }

        // /// Write a BGR pixel into the framebuffer
        // unsafe fn write_pixel_bgr(framebuffer: &mut FrameBuffer, pixel_base: usize, rgb: [u8; 3]) {
        //     framebuffer.write_value(pixel_base, [rgb[2], rgb[1], rgb[0]])
        // }

        // let func: PixelWriter = match pixel_format {
        //     PixelFormat::Rgb => write_pixel_rgb,
        //     PixelFormat::Bgr => write_pixel_bgr,
        //     _ => panic!("This pixel format is not support by the drawer"),
        // };

        // unsafe {
        //     let mut framebuffer = VIDEO_MANAGER.get_framebuffer();
        //     func(&mut framebuffer, pixel_base, rgb);
        // }
    }

    /// Draw a rectangle in a solid color.
    fn fillrect(&self, x: u32, y: u32, width: usize, height: usize, rbg: u32) {
        unimplemented!("Implement this function");
        // let current_mode = self.get_current_mode();

        // if x == 0 && width == current_mode.width && (rbg == 0 || rbg == 0xffffff) {
        //     unsafe {
        //         unimplemented!("Implement framebuffer get function");
        //         // let mut framebuffer = VIDEO_MANAGER.get_framebuffer();

        //         // let offset = fb_offset(x as usize, y as usize);
        //         // let base_addr = framebuffer.as_mut_ptr().add(offset);
        //         // let size = (height * current_mode.stride) * 4;
        //         // memset(base_addr, rbg as i32, size);
        //     }
        // } else {
        //     unimplemented!("TODO: implement a pixel by pixel approach for the other cases")
        // }
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

impl ConsoleOut for FramebufferConsole {
    fn init(&mut self) {
        todo!()
    }

    fn deinit(&mut self) {
        todo!()
    }

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
        todo!()
    }

    fn get_region(&self) -> DrawRegion {
        todo!()
    }

    fn set_cursor(&mut self, cursor: Cursor) {
        todo!()
    }

    fn get_cursor(&self) -> Cursor {
        todo!()
    }

    fn clear(&mut self, x: usize, y: usize, width: usize, height: usize) {
        todo!()
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
}
