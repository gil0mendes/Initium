/// A globally unique identifier.
///
/// GUIDs are used by to identify protocols and other objects.
/// 
/// The difference from regular UUIDs is that first 3 fields
/// are always encoded as little endian.
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(C)]
pub struct Guid {
    /// The low field of the timestamp.
    a: u32,
    /// The middle field of the timestamp.
    b: u16,
    /// The high field of the timestamp multiplexed with
    /// the version variant.
    c: u16,
    /// Contains:
    /// - The high field of the clock sequence
    /// multiplexed with the variant.
    /// - The low field of the clock sequence.
    /// - Spatially unique node identifier.
    d: [u8; 8],
}

impl Guid {
    /// Creates a new GUID from its components values.
    pub const fn from_values(a: u32, b: u16, c: u16, d: [u8; 8]) -> Self {
        Guid {
            a,
            b,
            c,
            d
        }
    }
}
