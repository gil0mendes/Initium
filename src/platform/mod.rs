//! Platform code

#[cfg(target_arch = "x86_64")]
pub mod efi;

#[cfg(target_arch = "x86_64")]
pub use self::efi::*;
