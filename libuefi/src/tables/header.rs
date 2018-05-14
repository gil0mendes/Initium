use super::Revision;

/// All standard UEFI tables starts with this header.
#[derive(Debug)]
#[repr(C)]
pub struct Header {
    /// Unique identifier for this table.
    pub signature: u64,
    /// Revision of this table.
    pub revision: Revision,
    /// Size in bytes of the entire table.
    pub size: u32,
    /// 32-bit CRC-32-Castagnoli of the entire table.
    pub crc: u32,
    /// Reserved field that must be set to 0.
    _reserved: u32,
}
