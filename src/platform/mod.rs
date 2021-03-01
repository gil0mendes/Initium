//! Platform code

#[cfg(target_arch = "x86_64")]
#[macro_use]
pub mod efi;

#[cfg(target_arch = "x86_64")]
pub use self::efi::*;
