#![no_std]
#![no_main]
#![feature(allocator_api)]
#![feature(const_fn)]
#![feature(custom_attribute)]
#![feature(optin_builtin_traits)]

#[macro_use]
extern crate bitflags;

pub mod types;
pub mod system_table;
pub mod protocols;
// pub mod boot_services;

// mod memory_services;
// mod miscellaneous;

pub use types::{Status, Guid, Result};

// pub use self::memory_services::types::*;
// pub use self::memory_services::{allocate_pages, free_pages, MemoryMap, MemoryMapFailure,
//                                 get_memory_map};
// pub use self::miscellaneous::set_watchdog_timer;
