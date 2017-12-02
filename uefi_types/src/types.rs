/// This type is not yet implemented. You are encouraged to submit a pull request.
pub type NotImplemented = *const ();

#[derive(Debug, Clone, PartialEq, Eq)]
#[repr(C)] // align 64-bit
pub struct Guid(pub u64, pub u64);

/// Status code returned by an EFI function.
pub type Status = usize;

/// The operation completed successfully.
pub const STATUS_SUCCESS: Status = 0;

/// The image failed to load.
pub const STATUS_LOAD_ERROR: Status = (1<<63) | 1;

/// A parameter was incorrect.
pub const STATUS_INVALID_PARAMETER: Status = (1<<63) | 2;

/// The operation is not supported.
pub const STATUS_UNSUPPORTED: Status = (1<<63) | 3;

/// The buffer was not the proper size for the request.
pub const STATUS_BAD_BUFFER_SIZE: Status = (1<<63) | 4;

/// The buffer is not large enough to hold the requested data. The required buffer size is
/// returned in the appropriate parameter when this error occurs.
pub const STATUS_BUFFER_TOO_SMALL: Status = (1<<63) | 5;

/// There is no data pending upon return.
pub const STATUS_NOT_READY: Status = (1<<63) | 6;

/// The physical device reported an error while attempting the operation.
pub const STATUS_DEVICE_ERROR: Status = (1<<63) | 7;

/// The device cannot be written to.
pub const STATUS_WRITE_PROTECTED: Status = (1<<63) | 8;

/// A resource has run out.
pub const STATUS_OUT_OF_RESOURCES: Status = (1<<63) | 9;

/// An inconstancy was detected on the file system causing the operating to fail.
pub const STATUS_VOLUME_CORRUPTED: Status = (1<<63) | 10;

/// There is no more space on the file system.
pub const STATUS_VOLUME_FULL: Status = (1<<63) | 11;

/// The device does not contain any medium to perform the operation.
pub const STATUS_NO_MEDIA: Status = (1<<63) | 12;

/// The medium in the device has changed since the last access.
pub const STATUS_MEDIA_CHANGED: Status = (1<<63) | 13;

/// The item was not found.
pub const STATUS_NOT_FOUND: Status = (1<<63) | 14;

/// Access was denied.
pub const STATUS_ACCESS_DENIED: Status = (1<<63) | 15;

/// The server was not found or did not respond to the request.
pub const STATUS_NO_RESPONSE: Status = (1<<63) | 16;

/// A mapping to a device does not exist.
pub const STATUS_NO_MAPPING: Status = (1<<63) | 17;

/// The timeout time expired.
pub const STATUS_TIMEOUT: Status = (1<<63) | 18;

/// The protocol has not been started.
pub const STATUS_NOT_STARTED: Status = (1<<63) | 19;

/// The protocol has already been started.
pub const STATUS_ALREADY_STARTED: Status = (1<<63) | 20;

/// The operation was aborted.
pub const STATUS_ABORTED: Status = (1<<63) | 21;

/// An ICMP error occurred during the network operation.
pub const STATUS_ICMP_ERROR: Status = (1<<63) | 22;

/// A TFTP error occurred during the network operation.
pub const STATUS_TFTP_ERROR: Status = (1<<63) | 23;

/// A protocol error occurred during the network operation.
pub const STATUS_PROTOCOL_ERROR: Status = (1<<63) | 24;

/// The function encountered an internal version that was incompatible with a version requested by
/// the caller.
pub const STATUS_INCOMPATIBLE_VERSION: Status = (1<<63) | 25;

/// The function was not performed due to a security violation.
pub const STATUS_SECURITY_VIOLATION: Status = (1<<63) | 26;

/// A CRC error was detected.
pub const STATUS_CRC_ERROR: Status = (1<<63) | 27;

/// Beginning or end of media was reached
pub const STATUS_END_OF_MEDIA: Status = (1<<63) | 28;

/// The end of the file was reached.
pub const STATUS_END_OF_FILE: Status = (1<<63) | 31;

/// The language specified was invalid.
pub const STATUS_INVALID_LANGUAGE: Status = (1<<63) | 32;

/// The security status of the data is unknown or compromised and the data must be updated or
/// replaced to restore a valid security status.
pub const STATUS_COMPROMISED_DATA: Status = (1<<63) | 33;

/// There is an address conflict address allocation
pub const STATUS_IP_ADDRESS_CONFLICT: Status = (1<<63) | 34;

/// A HTTP error occurred during the network operation.
pub const STATUS_HTTP_ERROR: Status = (1<<63) | 35;

/// The string contained one or more characters that the device could not render and were skipped.
pub const STATUS_WARN_UNKNOWN_GLYPH: Status = 1;

/// The handle was closed, but the file was not deleted.
pub const STATUS_WARN_DELETE_FAILURE: Status = 2;

/// The handle was closed, but the data to the file was not flushed properly.
pub const STATUS_WARN_WRITE_FAILURE: Status = 3;

/// The resulting buffer was too small, and the data was truncated to the buffer size.
pub const STATUS_WARN_BUFFER_TOO_SMALL: Status = 4;

/// The data has not been updated within the timeframe set by local policy for this type of data.
pub const STATUS_WARN_STALE_DATA: Status = 5;

/// The resulting buffer contains UEFI-compliant file system.
pub const STATUS_WARN_FILE_SYSTEM: Status = 6;

/// The operation will be processed across a system reset.
pub const STATUS_WARN_RESET_REQUIRED: Status = 7;

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
