//! Protocol definitions.
//!
//! Protocols are sets of related functionality.
//!
//! Protocols are identified by a unique ID.
//!
//! Protocols can be implemented by a UEFI drive, and a are
//! usually retrieved from a standard UEFI table or by 
//! querying a handle.

use super::types::Guid;

pub trait Protocol {
    /// Unique protocol identifier.
    const Guid: Guid;
}

#[macro_use]
mod macros;

pub mod console;
