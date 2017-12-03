use types::*;

/// Sets the system’s watchdog timer.
pub type SetWatchdogTimer = unsafe extern "win64" fn(
  // The number of seconds to set the watchdog timer to. A value 
  // of zero disables the timer.
  timeout: usize,
  // The numeric code to log on a watchdog timer timeout event. 
  // The firmware reserves codes 0x0000 to 0xFFFF. Loaders and 
  // operating systems may use other timeout codes.
  watchdogCode: u64,
  // The size, in bytes, of `watchdogCode`.
  dataSize: usize,
  // A data buffer that includes a Null-terminated string, 
  // optionally followed by additional binary data. The string is 
  // a description that the call may use to further indicate the 
  // reason to be logged with a watchdog event.
  watchdogData: *mut u16
) -> Status;

///  Copies the contents of one buffer to another buffer.
pub type CopyMem = unsafe extern "win64" fn(
  // DestinationPointer to the destination buffer of the 
  // memory copy.
  destinationPointer: *mut usize,
  // SourcePointer to the source buffer of the memory copy.
  sourcePointer: *const usize,
  // LengthNumber of bytes to copy from Source to Destination.
  lengthNumber: u64
) -> Status;

/// Induces a fine-grained stall.
pub type Stall = unsafe extern "win64" fn(
  // The number of microseconds to stall execution.
  microseconds: usize
) -> Status;
