use core::fmt::{self, Write};

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::print::write_fmt(format_args!($($arg)*)).unwrap());
}

#[macro_export]
macro_rules! println {
    () => (print!("\n"));
    ($fmt:expr) => (print!(concat!($fmt, "\n")));
    ($fmt:expr, $($arg:tt)*) => (print!(concat!($fmt, "\n"), $($arg)*));
}

pub fn write_fmt(args: fmt::Arguments) -> fmt::Result {
    use uefi::SimpleTextOutputProtocol as Console;
    Write::write_fmt(unsafe { &mut *(::UEFI_SYSTEM_TABLE.unwrap().get_console_out() as *const Console as *mut Console) }, args)
}
