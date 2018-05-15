use {Status, Result};

/// Interface for text-based input devices.
#[repr(C)]
pub struct Input {
    reset: extern "C" fn(this: &mut Input, extended: bool) -> Status,
    read_key_stroke: extern "C" fn(this: &mut Input, key: &mut Key) -> Status,
}

/// A key read from the console.
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(C)]
pub struct Key {
    /// The scanned key code
    pub scan_code: ScanCode,
    /// Associated Unicode character or 0 if not printable.
    pub unicode_char: u16,
}

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(u16)]
pub enum ScanCode {
    Null,
    Up,
    Down,
    Right,
    Left,
    Home,
    End,
    Insert,
    Delete,
    PageUp,
    PageDown,
    Function1,
    Function2,
    Function3,
    Function4,
    Function5,
    Function6,
    Function7,
    Function8,
    Function9,
    Function10,
    Function11,
    Function12,
    Escape,
    Function13 = 0x68,
    Function14,
    Function15,
    Function16,
    Function17,
    Function18,
    Function19,
    Function20,
    Function21,
    Function22,
    Function23,
    Function24,
    Mute = 0x7F,
    VolumeUp = 0x80,
    VolumeDown,
    BrightnessUp = 0x100,
    BrightnessDown,
    Suspend,
    Hibernate,
    ToggleDisplay,
    Recovery,
    Eject,
}

impl_proto! {
    protocol Input {
        GUID = 0x387477c1, 0x69c7, 0x11d2, [0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b];
    }
}
