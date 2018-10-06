//! Initium is a modern bootloader with no legacy components.

#![no_std]
#![no_main]

extern crate platform;

mod config;

#[no_mangle]
pub extern fn load_main() {
    loop {}
}
