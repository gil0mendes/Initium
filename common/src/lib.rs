#![no_std]

use alloc::string::String;
use alloc::vec::Vec;
use core::ptr::NonNull;

pub mod console;
pub mod font;
pub mod key;
pub mod video;

extern crate alloc;

/// Type for a command function
pub type CommandFn = fn(Vec<String>) -> bool;

pub struct BuiltinCommand<'a> {
    /// Name of the command
    pub name: &'a str,
    /// Description of the command
    pub description: &'a str,
    /// Command function
    pub func: CommandFn,
}
