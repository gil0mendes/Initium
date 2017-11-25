#![no_std]
#![no_main]

#![feature(start)]

#[start]
#[no_mangle]
pub extern fn efi_main() {
  loop {
  }
}
