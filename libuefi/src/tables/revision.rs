use core::fmt;

/// Struct to represent the UEFI revision.
///
/// The minor revision number is incremented on minor changes and is represented as two-digit
/// binary-coded decimal.
#[derive(Copy, Clone, Eq, PartialOrd, PartialEq, Ord)]
pub struct Revision(u32);

impl Revision {
    /// Create a new revision instance.
    pub fn new(major: u16, minor: u16) -> Self {
        let (major, minor) = (major as u32, minor as u32);
        let value = (major << 16) | minor;
        Revision(value)
    }

    /// Returns the major revision.
    pub fn major(&self) -> u16 {
        (self.0 >> 16) as u16
    }

    /// Returns the minor revision.
    pub fn minor(&self) -> u16 {
        self.0 as u16
    }
}

impl fmt::Debug for Revision {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let (major, minor) = (self.major(), self.minor());
        write!(f, "{}.{}.{}", major, minor / 10, minor % 10)
    }
}
