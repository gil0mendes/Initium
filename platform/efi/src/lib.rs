#![no_std]
#![no_main]
#![feature(custom_attribute)]

extern crate uefi_types;

use uefi_types::system_table::SystemTable;

/// Entry point for EFI platforms
#[no_mangle]
#[no_stack_check]
pub extern fn efi_main(image_handle: usize, system_table: SystemTable) {
  loop {
  }
}
