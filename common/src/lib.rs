#![no_std]

use alloc::string::String;
use alloc::vec::Vec;
use core::ptr::NonNull;

pub use command_manager::BuiltinCommand;

pub mod command_manager;
pub mod console;
pub mod font;
pub mod key;
pub mod video;

extern crate alloc;
