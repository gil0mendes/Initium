#![no_std]

pub use command_manager::BuiltinCommand;

pub mod command_manager;
pub mod console;
pub mod key;
pub mod video;

extern crate alloc;
