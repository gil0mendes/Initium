mod status;

use core::alloc::Opaque;
use core::result;
pub use self::status::Status;

/// Return type of many UEFI functions.
pub type Result<T> = result::Result<T, Status>;

/// A pointer to an opaque data structure.
pub type Handle = *mut Opaque;

/// This type is not yet implemented. You are encouraged to submit a pull request.
pub type NotImplemented = *const Opaque;
