use super::Result;

/// Status codes are returned by UEFI interfaces to indicate whether an
/// operation completed successfully.
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
#[repr(usize)]
pub enum Status {
    /// The operation completed successfully.
     Success = 0,
    /// The image failed to load.
    LoadError = (1<<63) | 1,
    /// A parameter was incorrect.
    InvalidParameter = (1<<63) | 2,
    /// The operation is not supported.
    Unsupported = (1<<63) | 3,
    /// The buffer was not the proper size for the request.
    BadBufferSize = (1<<63) | 4,
    /// The buffer is not large enough to hold the requested data. The required buffer size is
    /// returned in the appropriate parameter when this error occurs.
    BufferTooSmall = (1<<63) | 5,
    /// There is no data pending upon return.
    NotReady = (1<<63) | 6,
    /// The physical device reported an error while attempting the operation.
    DeviceError = (1<<63) | 7,
    /// The device cannot be written to.
    WriteProtected = (1<<63) | 8,
    /// A resource has run out.
    OutOfResources = (1<<63) | 9,
    /// An inconstancy was detected on the file system causing the operating to fail.
    VolumeCorrupted = (1<<63) | 10,
    /// There is no more space on the file system.
    VolumeFull = (1<<63) | 11,
    /// The device does not contain any medium to perform the operation.
    NoMedia = (1<<63) | 12,
    /// The medium in the device has changed since the last access.
    MediaChanged = (1<<63) | 13,
    /// The item was not found.
    NotFound = (1<<63) | 14,
    /// Access was denied.
    AccessDinied = (1<<63) | 15,
    /// The server was not found or did not respond to the request.
    NoResponse = (1<<63) | 16,
    /// A mapping to a device does not exist.
    NoMapping = (1<<63) | 17,
    /// The timeout time expired.
    Timeout = (1<<63) | 18,
    /// The protocol has not been started.
    NotStarted = (1<<63) | 19,
    /// The protocol has already been started.
    AlreadyStarted = (1<<63) | 20,
    /// The operation was aborted.
    Aborted = (1<<63) | 21,
    /// An ICMP error occurred during the network operation.
    ICMPError = (1<<63) | 22,
    /// A TFTP error occurred during the network operation.
    TFTPError = (1<<63) | 23,
    /// A protocol error occurred during the network operation.
    ProtocolError = (1<<63) | 24,
    /// The function encountered an internal version that was incompatible with a version requested by
    /// the caller.
    IncompatibleVersion = (1<<63) | 25,
    /// The function was not performed due to a security violation.
    SecurityViolation = (1<<63) | 26,
    /// A CRC error was detected.
    CRCError = (1<<63) | 27,
    /// Beginning or end of media was reached
    EndOfMedia = (1<<63) | 28,
    /// The end of the file was reached.
    EndOfFile = (1<<63) | 31,
    /// The language specified was invalid.
    InvalidLanguage = (1<<63) | 32,
    /// The security status of the data is unknown or compromised and the data must be updated or
    /// replaced to restore a valid security status.
    CompromisedData = (1<<63) | 33,
    /// There is an address conflict address allocation
    IPAddressConflict = (1<<63) | 34,
    /// A HTTP error occurred during the network operation.
    HTTPError = (1<<63) | 35,
    /// The string contained one or more characters that the device could not render and were skipped.
    WarnUnknownGlyph = 1,
    /// The handle was closed, but the file was not deleted.
    WarnDeleteFailure = 2,
    /// The handle was closed, but the data to the file was not flushed properly.
    WarnWriteFailure = 3,
    /// The resulting buffer was too small, and the data was truncated to the buffer size.
    WarnBufferTooSmall = 4,
    /// The data has not been updated within the timeframe set by local policy for this type of data.
    WarnStaleData = 5,
    /// The resulting buffer contains UEFI-compliant file system.
    WarnFileSystem = 6,
    /// The operation will be processed across a system reset.
    WarnResetRequired = 7,
}

impl Status {
    /// Returns true if status code indicates success.
    #[inline]
    pub fn is_success(self) -> bool {
        self == Status::Success
    }

    pub fn into_with<T, F>(self, f: F) -> Result<T>
    where F: FnOnce() -> T {
        if self.is_success() {
            Ok(f())
        } else {
            Err(self)
        }
    }
}

impl Into<Result<()>> for Status {
    #[inline]
    fn into(self) -> Result<()> {
        self.into_with(|| ())
    }
}
