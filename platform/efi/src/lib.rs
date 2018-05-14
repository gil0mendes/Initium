#![feature(lang_items)]
#![feature(compiler_builtins_lib)]
#![feature(try_trait)]
#![no_std]
#![no_main]

extern crate libuefi;

use libuefi::types::*;
use libuefi::SystemTable;

// pub(crate) static mut UEFI_SYSTEM_TABLE: Option<&'static uefi::SystemTable> = None;

extern {
    fn load_main();
}

/// Entry point for EFI platforms
#[no_mangle]
pub extern "win64" fn uefi_start(image_handle: Handle, system_table: &'static SystemTable) -> Status {
    system_table.stdout().reset(false);

    unsafe { load_main(); }

    Status::Success
    // unsafe {
    //     UEFI_SYSTEM_TABLE = Some(&system_table);
    // }
    // println!("UEFI header: {:#?}", system_table.get_header());
    // main();
    // uefi::Status::Success
}

// fn main() {
//     println!("Hello, {}!", "UEFI world");
//     println!("Version: {}", env!("CARGO_PKG_VERSION"));
//     println!("Authors:");
//     for author in env!("CARGO_PKG_AUTHORS").split(';') {
//         println!("    {}", author);
//     }
// }

#[lang = "panic_fmt"]
#[no_mangle]
pub extern fn rust_begin_panic(message: core::fmt::Arguments, file: &'static str, line: u32, column: u32) -> ! {
    // println!("Panic in {file} at {line}:{column}: {message}", message=message, file=file, line=line, column=column);
    loop {}
}

#[no_mangle]
pub extern "C" fn __floatundisf() {
    loop {}
}
