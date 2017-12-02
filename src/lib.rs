//! Initium is a modern bootloader with no legacy components.

#![feature(lang_items)]
#![no_std]

extern crate platform;

#[cfg(not(test))]
#[lang = "panic_fmt"]
#[no_mangle]
pub extern "C" fn panic_fmt(fmt: core::fmt::Arguments, file: &'static str, line: u32) -> ! {
    loop {}
}
