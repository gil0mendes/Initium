#![no_std]
#![no_main]

#![feature(compiler_builtins_lib)]
#![feature(panic_implementation)]
#![feature(try_trait)]

extern crate uefi;

use core::panic::PanicInfo;

use uefi::prelude::*;

// pub(crate) static mut UEFI_SYSTEM_TABLE: Option<&'static uefi::SystemTable> = None;

/// Entry point for EFI platforms
#[no_mangle]
pub extern "C" fn uefi_start(_image_handle: uefi::Handle, system_table: &'static SystemTable) -> Status {
    // Initialize logging.
    uefi_services::init(system_table);

    loop {}

    // unsafe { load_main(); }

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

// #[panic_handler]
// fn panic(_info: &PanicInfo) -> ! {
//     // println!("Panic in {file} at {line}:{column}: {message}", message=message, file=file, line=line, column=column);
//     loop {}
// }
