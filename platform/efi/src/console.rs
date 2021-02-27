use crate::{video::VIDEO_MANAGER, SYSTEM_TABLE};
use alloc::boxed::Box;
use alloc::vec;
use alloc::vec::Vec;
use common::{
    console::ConsoleIn,
    key::{Key, ScanCode},
    video::{FrameBuffer, PixelFormat, VideoMode},
};
use common::{
    console::{Char, Color, ConsoleOut, DrawRegion, FONT_HEIGHT, FONT_WIDTH},
    font::CONSOLE_FONT,
};
use core::fmt;
use core::result::Result;
use rlibc::memset;
use uefi::{
    prelude::BootServices,
    proto::console::text::{
        self, Input,
        Key::{Printable, Special},
    },
    Char16,
};

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
        let mut console = &mut CONSOLE_OUT;
        console.as_mut().unwrap().write_fmt(args)
    }
}

/// Type for the cursor position
type CursorPos = (usize, usize);

pub static mut CONSOLE_OUT: Option<ConsoleOutManager> = None;

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
    /// Cursor is enabled
    cursor_visible: bool,
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
            cursor_visible: true,
            chars: Box::new(Vec::new()),
            foreground_color: Color::White,
            background_color: Color::Black,
        };

        unsafe {
            CONSOLE_OUT = Some(manager);
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
            PixelFormat::Rgb => write_pixel_rgb,
            PixelFormat::Bgr => write_pixel_bgr,
            _ => panic!("This pixel format is not support by the drawing"),
        };

        unsafe {
            let mut video_manager = VIDEO_MANAGER.unwrap();
            let mut framebuffer = video_manager.get_framebuffer();
            func(&mut framebuffer, pixel_base, rgb);
        }
    }

    /// Scroll the draw region down (does not change the cursor).
    fn scroll_down(&mut self) {
        unimplemented!("TODO: implement scroll down");
    }

    fn putc(&mut self, ch: char) {
        self.toggle_cursor();

        match ch {
            // backspace, move back one character of we can
            '\x08' => {
                if self.cursor_pos.0 > self.region.x {
                    self.cursor_pos.0 = self.cursor_pos.0 - 1;
                } else if self.cursor_pos.1 > self.region.y {
                    self.cursor_pos =
                        (self.region.x + self.region.width - 1, self.cursor_pos.1 - 1);
                }
            }
            // carriage return, move to the start of the line
            '\r' => {
                self.cursor_pos.0 = self.region.x;
            }
            // newline, treat it as if a carriage return was there
            '\n' => {
                self.cursor_pos = (self.region.x, self.cursor_pos.1 + 1);
            }
            // tab
            '\t' => {
                self.cursor_pos.0 = self.cursor_pos.0 + 8 - (self.cursor_pos.0 % 8);
            }
            // only deal with printable chars
            _ => {
                if ch >= ' ' {
                    let idx = (self.cursor_pos.1 * self.cols) + self.cursor_pos.0;
                    self.chars[idx].char = ch;
                    self.chars[idx].foreground = self.foreground_color;
                    self.chars[idx].background = self.background_color;

                    self.draw_glyph(self.cursor_pos.0, self.cursor_pos.1);

                    self.cursor_pos.0 = self.cursor_pos.0 + 1;
                }
            }
        }

        let (x, y) = self.cursor_pos;

        // if we have reached the edge of the draw region insert a new line
        if (x >= self.region.x + self.region.width) {
            self.cursor_pos = (self.region.x, y + 1);
        }

        // if we have reached the bottom of the draw region, scroll
        if y > self.region.y + self.region.height {
            if self.region.scrollable {
                self.scroll_down();
            }

            // update the cursor position
            self.cursor_pos.1 = self.region.y + self.region.height - 1;
        }

        self.toggle_cursor();
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
            unimplemented!("TODO: implement a pixel by pixel approach for the other cases")
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

    /// Toggle the cursor if enabled.
    fn toggle_cursor(&mut self) {
        if !self.cursor_visible {
            return;
        }

        let idx = (self.cursor_pos.1 * self.cols) + self.cursor_pos.0;

        if (self.chars[idx].char as u8 > 0) {
            // invert the colors
            let temp = self.chars[idx].foreground;
            self.chars[idx].foreground = self.chars[idx].background;
            self.chars[idx].background = temp;
        } else {
            // nothing has yet been written, initialize the character. we must be enabling the cursor if this is the
            // case, so invert colors.
            self.chars[idx].char = ' ';
            self.chars[idx].foreground = Color::Black;
            self.chars[idx].background = Color::Light_grey;
        }

        // redraw glyph with the new colors
        self.draw_glyph(self.cursor_pos.0, self.cursor_pos.1);
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

        self.fillrect(0, 0, current_mode.width, current_mode.height, 0x00);

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

                if self.cursor_visible && abs_x == self.cursor_pos.0 && abs_y == self.cursor_pos.1 {
                    // avoid redrawing the glyph twice when cursor is active
                    self.toggle_cursor();
                } else {
                    self.draw_glyph(abs_x, abs_y);
                }
            }
        }
    }

    fn set_region(&mut self, region: DrawRegion) {
        assert!(region.width > 0 && region.height > 0);
        assert!(region.x + region.width <= self.cols);
        assert!(region.y + region.height <= self.rows);

        self.region = region;

        // adjust cursor position
        self.set_cursor(0, 0, self.cursor_visible);
    }

    fn get_region(&self) -> DrawRegion {
        self.region.clone()
    }

    fn reset_region(&mut self) {
        let region = DrawRegion {
            x: 0,
            y: 0,
            width: self.cols,
            height: self.rows,
            scrollable: true,
        };
        self.set_region(region);
    }

    fn set_color(&mut self, fg: common::console::Color, bg: common::console::Color) {
        self.foreground_color = fg;
        self.background_color = bg;
    }

    fn resolution(&self) -> (usize, usize) {
        (self.cols, self.rows)
    }

    fn set_cursor(&mut self, x: usize, y: usize, visible: bool) {
        assert!(x < self.region.width);
        assert!(y < self.region.height);

        self.toggle_cursor();
        self.cursor_pos = (self.region.x + x, self.region.y + y);
        self.cursor_visible = visible;
        self.toggle_cursor();
    }
}

impl fmt::Write for ConsoleOutManager {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        s.chars().for_each(|c| self.putc(c));

        Result::Ok(())
    }
}

/**
 * Console output logic
 * TODO: this is temporary, we need to find a better way to make this more scalable using RUST, on C this was easy 😅
 */

/// Console input device
pub static mut CONSOLE_IN: Option<ConsoleInDevice> = None;

/// EFI console input device
pub struct ConsoleInDevice<'a> {
    /// text input protocol
    efi_input_proto: &'a mut Input,
    /// key saved when using a poll
    saved_key: Option<text::Key>,
}

impl<'a> ConsoleInDevice<'a> {
    /// Initialize the input device
    pub fn init(bt: &BootServices) {
        use crate::uefi::ResultExt;

        // Look for a text input handler
        let mut text_proto = bt
            .locate_protocol::<Input>()
            .expect_success("UEFI Text Input Protocol is not supported");

        let input = unsafe { &mut *text_proto.get() };

        let device = ConsoleInDevice {
            saved_key: None,
            efi_input_proto: input,
        };

        unsafe {
            CONSOLE_IN = Some(device);
        }
    }
}

impl ConsoleIn for ConsoleInDevice<'a> {
    fn poll(&mut self) -> bool {
        if (self.saved_key.is_some()) {
            return true;
        }

        while (true) {
            // read a key from the console
            let key = match self.efi_input_proto.read_key() {
                Ok(result) => {
                    let maybe_key = result.unwrap();
                    if maybe_key.is_none() {
                        continue;
                    }

                    maybe_key.unwrap()
                }
                Err(_) => {
                    return false;
                }
            };

            self.saved_key = Some(key);
            return true;
        }

        return true;
    }

    fn get_char(&mut self) -> Key {
        if (self.saved_key.is_none()) {
            // wait the user hit a key
            while (!self.poll()) {}
        }

        // convert UEFI key into a key understood by the common code
        let to_return = convert_uefi_key_into_key(self.saved_key.unwrap());
        self.saved_key = None;
        to_return
    }
}

/// Convert UEFI key into
fn convert_uefi_key_into_key(key: text::Key) -> Key {
    match key {
        text::Key::Printable(unicode_char) => {
            let unicode_u16: u16 = unicode_char.into();
            if unicode_u16 == '\r' as u16 {
                return Key::Printable('\n');
            }

            return Key::Printable(unicode_u16 as u8 as char);
        }
        text::Key::Special(special_key) => match special_key {
            text::ScanCode::NULL => Key::Special(ScanCode::NULL),
            text::ScanCode::UP => Key::Special(ScanCode::UP),
            text::ScanCode::DOWN => Key::Special(ScanCode::DOWN),
            text::ScanCode::RIGHT => Key::Special(ScanCode::RIGHT),
            text::ScanCode::LEFT => Key::Special(ScanCode::LEFT),
            text::ScanCode::HOME => Key::Special(ScanCode::HOME),
            text::ScanCode::END => Key::Special(ScanCode::END),
            text::ScanCode::INSERT => Key::Special(ScanCode::INSERT),
            text::ScanCode::DELETE => Key::Special(ScanCode::DELETE),
            text::ScanCode::PAGE_UP => Key::Special(ScanCode::PAGE_UP),
            text::ScanCode::PAGE_DOWN => Key::Special(ScanCode::PAGE_DOWN),
            text::ScanCode::FUNCTION_1 => Key::Special(ScanCode::FUNCTION_1),
            text::ScanCode::FUNCTION_2 => Key::Special(ScanCode::FUNCTION_2),
            text::ScanCode::FUNCTION_3 => Key::Special(ScanCode::FUNCTION_3),
            text::ScanCode::FUNCTION_4 => Key::Special(ScanCode::FUNCTION_4),
            text::ScanCode::FUNCTION_5 => Key::Special(ScanCode::FUNCTION_5),
            text::ScanCode::FUNCTION_6 => Key::Special(ScanCode::FUNCTION_6),
            text::ScanCode::FUNCTION_7 => Key::Special(ScanCode::FUNCTION_7),
            text::ScanCode::FUNCTION_8 => Key::Special(ScanCode::FUNCTION_8),
            text::ScanCode::FUNCTION_9 => Key::Special(ScanCode::FUNCTION_9),
            text::ScanCode::FUNCTION_10 => Key::Special(ScanCode::FUNCTION_10),
            text::ScanCode::FUNCTION_11 => Key::Special(ScanCode::FUNCTION_11),
            text::ScanCode::FUNCTION_12 => Key::Special(ScanCode::FUNCTION_12),
            text::ScanCode::ESCAPE => Key::Special(ScanCode::ESCAPE),
            // for non supported keys return NULL
            _ => Key::Special(ScanCode::NULL),
        },
    }
}
