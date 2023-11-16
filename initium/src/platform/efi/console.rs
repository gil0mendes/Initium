use crate::platform::allocator::boot_services;

use common::{
    console::ConsoleIn,
    key::{Key, ScanCode},
};
use core::fmt;
use core::result::Result;
use uefi::{
    proto::console::text::{self, Input},
    table::boot::ScopedProtocol,
};

// pub fn print_fmt(args: fmt::Arguments) -> fmt::Result {
//     use core::fmt::Write;

//     unsafe {
//         let mut console = CONSOLE_OUT.expect("The platform console was not initialized");
//         console.as_mut().write_fmt(args)
//     }
// }

// impl ConsoleOut for PlatformConsole {
//     fn init(&mut self) {
//         let current_mode = self.get_current_mode();

//         self.cols = current_mode.width / FONT_WIDTH;
//         self.rows = current_mode.height / FONT_HEIGHT;

//         // initialize chars array and zero the vector
//         let size = self.cols * self.rows;
//         self.chars = Box::new(vec![Char::default(); size as usize]);

//         self.fillrect(0, 0, current_mode.width, current_mode.height, 0x00);

//         self.region = DrawRegion {
//             x: 0,
//             y: 0,
//             width: current_mode.width,
//             height: current_mode.height,
//             scrollable: false,
//         }
//     }

//     fn clear(&mut self, x: usize, y: usize, mut width: usize, mut height: usize) {
//         use common::video::VideoManager;

//         assert!(x + width <= self.region.width);
//         assert!(y + height <= self.region.height);

//         let stride = unsafe { VIDEO_MANAGER.unwrap().get_mode().stride };

//         if width == 0 {
//             width = self.region.width - x;
//         }
//         if height == 0 {
//             height = self.region.height - y;
//         }

//         for row in 0..height {
//             for col in 0..width {
//                 let abs_x = self.region.x + col;
//                 let abs_y = self.region.y + row;
//                 let idx = (abs_y * self.cols) + abs_x;

//                 self.chars[idx].char = ' ';
//                 self.chars[idx].foreground = self.foreground_color;
//                 self.chars[idx].background = self.background_color;

//                 if self.cursor_visible && abs_x == self.cursor_pos.0 && abs_y == self.cursor_pos.1 {
//                     // avoid redrawing the glyph twice when cursor is active
//                     self.toggle_cursor();
//                 } else {
//                     self.draw_glyph(abs_x, abs_y);
//                 }
//             }
//         }
//     }

//     fn set_region(&mut self, region: DrawRegion) {
//         assert!(region.width > 0 && region.height > 0);
//         assert!(region.x + region.width <= self.cols);
//         assert!(region.y + region.height <= self.rows);

//         self.region = region;

//         // adjust cursor position
//         self.set_cursor(0, 0, self.cursor_visible);
//     }

//     fn get_region(&self) -> DrawRegion {
//         self.region.clone()
//     }

//     fn reset_region(&mut self) {
//         let region = DrawRegion {
//             x: 0,
//             y: 0,
//             width: self.cols,
//             height: self.rows,
//             scrollable: true,
//         };
//         self.set_region(region);
//     }

//     fn set_color(&mut self, fg: common::console::Color, bg: common::console::Color) {
//         self.foreground_color = fg;
//         self.background_color = bg;
//     }

//     fn set_cursor(&mut self, x: usize, y: usize, visible: bool) {
//         assert!(x < self.region.width);
//         assert!(y < self.region.height);

//         self.toggle_cursor();
//         self.cursor_pos = (self.region.x + x, self.region.y + y);
//         self.cursor_visible = visible;
//         self.toggle_cursor();
//     }
// }

// impl fmt::Write for PlatformConsole {
//     fn write_str(&mut self, s: &str) -> fmt::Result {
//         s.chars().for_each(|c| self.putc(c));

//         Result::Ok(())
//     }
// }

/**
 * Console output logic
 * TODO: this is temporary, we need to find a better way to make this more scalable using RUST, on C this was easy 😅
 */

/// Console input device
pub static mut CONSOLE_IN: Option<ConsoleInDevice> = None;

/// EFI console input device
pub struct ConsoleInDevice {
    /// text input protocol
    efi_input_proto: ScopedProtocol<'static, Input>,
    /// key saved when using a poll
    saved_key: Option<text::Key>,
}

impl ConsoleInDevice {
    /// Initialize the input device
    pub fn init() {
        let bt = unsafe { boot_services().as_ref() };

        if let Ok(handle) = bt.get_handle_for_protocol::<Input>() {
            let text_proto = bt
                .open_protocol_exclusive(handle)
                .expect("efi: Text Input Protocol is not supported");

            let device = ConsoleInDevice {
                saved_key: None,
                efi_input_proto: text_proto,
            };

            unsafe {
                CONSOLE_IN = Some(device);
            }
        } else {
            panic!("efi: no handle found for Text Input Protocol")
        }
    }
}

impl ConsoleIn for ConsoleInDevice {
    fn poll(&mut self) -> bool {
        if self.saved_key.is_some() {
            return true;
        }

        loop {
            // read a key from the console
            let key = match self.efi_input_proto.read_key() {
                Ok(Some(key)) => key,
                Ok(_) => continue,
                Err(_) => {
                    return false;
                }
            };

            self.saved_key = Some(key);
            return true;
        }
    }

    fn get_char(&mut self) -> Key {
        if self.saved_key.is_none() {
            // wait the user hit a key
            while !self.poll() {}
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

/// Initialize EFI console
pub fn init() {
    // TODO: initialize serial console

    ConsoleInDevice::init();
}
