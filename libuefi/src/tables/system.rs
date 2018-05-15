use types::*;
use protocols::console::text;
use super::header::Header;
use super::revision::Revision;
// use boot_services::BootServices;

// const SYSTEM_TABLE_SIGNATURE: u64 = 0x5453595320494249;
#[repr(C)]
pub struct SystemTable {
    /// Table header
    pub header: Header,
    /// Null-terminated UTF-16 name of firmware vendor
    pub firmware_vendor: *const u16, // char16
    /// Revision of firmware
    pub firmware_revision: Revision,
    /// Handle to the active console input device
    pub console_in_handle: Handle,
    /// Pointer to the interface associated with console_in_handle
    pub con_in: text::Input,
    /// Handle to the active console output device
    pub console_out_handle: Handle,
    /// Pointer to the interface associated with console_out_handle
    con_out: *mut text::Output,
    /// Handle to the active console standard error device
    pub standard_error_handle: Handle,
    /// Pointer to the interface associated with standard_error_handle
    pub std_err: NotImplemented,
    /// Pointer to RuntimeServices table
    pub runtime_services: NotImplemented,
    /// Pointer to BootServices table
    // pub boot_services: *const BootServices,
    pub boot_services: NotImplemented,
    /// The number of entries in the configuration_table (next field)
    pub number_of_table_entries: usize,
    /// A pointer to an array of system configuration table.
    pub configuration_table: NotImplemented,
}

impl SystemTable {
    /// Return the standard output protocol.
    pub fn stdout(&self) -> &mut text::Output {
        unsafe { &mut *self.con_out }
    }
}

impl super::Table for SystemTable {
    const SIGNATURE: u64 = 0x5453_5953_2049_4249;
}
