//! Standard UEFI tables.

/// Common trait implemented by all standard UEFI tables
pub trait Table {
    /// A unique number assigned by the UEFI specification to the standard tables.
    const SIGNATURE: u64;
}

mod revision;
mod header;
mod system;

pub use self::revision::Revision;
pub use self::system::SystemTable;
