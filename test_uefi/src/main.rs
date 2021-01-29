#![no_std]
#![no_main]
#![feature(global_asm)]
#![feature(abi_efiapi)]

global_asm!(include_str!("start.S"));

#[no_mangle]
extern "C" fn efi_main() {
    loop {}
}

#[panic_handler]
fn panic_handler(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}
