use {Status, Result};

/// Interface for text-based output devices.
#[repr(C)]
pub struct Output {
    reset: extern "C" fn(this: &Output, extended: bool) -> Status,
    output_string: extern "C" fn(this: &Output, string: *const u16) -> Status,
    test_string: extern "C" fn(this: &Output, *const u16) -> Status,
    query_mode: extern "C" fn(this: &Output,
                              mode: i32,
                              columns: &mut usize,
                              rows: &mut usize)
                              -> Status,
    set_mode: extern "C" fn(this: &Output, mode: i32) -> Status,
    set_attribute: extern "C" fn(this: &Output, attribute: i32) -> Status,
    clear_screen: extern "C" fn(this: &Output) -> Status,
    set_cursor_position: extern "C" fn(this: &Output, col: i32, row: i32) -> Status,
    enable_cursor: extern "C" fn(this: &Output, visible: bool) -> Status,
}

impl Output {
    /// Resets the text output device hardware.
    #[inline]
    pub fn reset(&mut self, extended: bool) -> Result<()> {
        (self.reset)(self, extended).into()
    }
}

impl_proto! {
    protocol Output {
        GUID = 0x387477c2, 0x69c7, 0x11d2, [0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b];
    }
}

// Reset OutputString
// TestString QueryMode
// SetMode SetAttribute
// ClearScreen
// Reset the ConsoleOut device. See Reset().
// Displays the string on the device at the current cursor location. See
// OutputString().
// Tests to see if the ConsoleOut device supports this string. See
// TestString().
// Queries information concerning the output device’s supported text
// mode. See QueryMode().
// Sets the current mode of the output device. See SetMode().
// Sets the foreground and background color of the text that is output. See SetAttribute().
// Clears the screen with the currently set background color. See ClearScreen().
