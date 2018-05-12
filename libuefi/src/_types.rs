/// This type is not yet implemented. You are encouraged to submit a pull request.
pub type NotImplemented = *const ();

#[derive(Debug, Clone, PartialEq, Eq)]
#[repr(C)] // align 64-bit
pub struct Guid(pub u64, pub u64);

/// Status code returned by an EFI function.
// pub type Status = usize;

pub type Handle = *mut ();

pub type Event = *mut ();

pub type Lba = u64;

pub type Tpl = usize;

#[derive(Debug, Clone, PartialEq, Eq)]
#[repr(C)]
pub struct MacAddress(pub [u8; 32]);

#[derive(Debug, Clone, PartialEq, Eq)]
#[repr(C)]
pub struct Ipv4Address(pub [u8; 4]);

#[derive(Debug, Clone, PartialEq, Eq)]
#[repr(C)]
pub struct Ipv6Address(pub [u8; 16]);

#[derive(Debug, Clone, PartialEq, Eq)]
#[repr(C)] // align 4-byte
pub struct IpAddress(pub [u8; 16]);

// for enums, use #[repr(u32)]

/// The header type for all table structures
#[derive(Debug, Clone)]
#[repr(C)]
pub struct TableHeader {
    /// Identifies the type of table that follows
    pub signature: u64,
    /// The revision of the EFI specification to which the table conforms (in an encoded
    /// sort of way)
    pub revision: u32,
    /// The size of the entire table, including the header (spec calls this HeaderSize but
    /// that is quite misleading)
    pub table_size: u32,
    /// The CRC32 for the entire table.  Compute by setting this field to 0 first, and use
    /// CCITT32 CRC with seed polynomial value of 0x04c11db7
    pub crc32: u32,
    _reserved: u32, // must be zero
}

impl TableHeader {
    pub fn get_revision_major(&self) -> u16
    {
        (self.revision >> 16) as u16
    }

    pub fn get_revision_minor(&self) -> u16
    {
        // the 'and' is not strictly necessary as rusts cast will truncate
        // (self.revision & 0x00ff) as u16
        self.revision as u16
    }
}
