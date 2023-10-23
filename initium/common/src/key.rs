///! Implementation of some components to work with keys

/// A key read from the console
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum Key {
    /// The key is associated with a printable character
    Printable(char),
    /// The key is special (arrow, function, multimedia...)
    Special(ScanCode),
}

/// A keyboard scan code
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum ScanCode {
    /// Null scan code, indicates that the Unicode character should be used.
    NULL,
    /// Move cursor up 1 row.
    UP,
    /// Move cursor down 1 row.
    DOWN,
    /// Move cursor right 1 column.
    RIGHT,
    /// Move cursor left 1 column.
    LEFT,
    HOME,
    END,
    INSERT,
    DELETE,
    PAGE_UP,
    PAGE_DOWN,
    FUNCTION_1,
    FUNCTION_2,
    FUNCTION_3,
    FUNCTION_4,
    FUNCTION_5,
    FUNCTION_6,
    FUNCTION_7,
    FUNCTION_8,
    FUNCTION_9,
    FUNCTION_10,
    FUNCTION_11,
    FUNCTION_12,
    ESCAPE,
}
