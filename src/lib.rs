//! Initium is a modern bootloader with no legacy components.

#![feature(lang_items)]
#![no_std]

extern crate platform;

#[no_mangle]
pub extern fn load_main() {
    loop {}
}
