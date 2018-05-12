use types::*;
// use boot_services::BootServices;

// const SYSTEM_TABLE_SIGNATURE: u64 = 0x5453595320494249;

pub struct SystemTable {
    /// Table header
    // pub hdr: TableHeader,
    pub hdr: NotImplemented,
    /// Null-terminated UTF-16 name of firmware vendor
    pub firmware_vendor: *const u16, // char16
    /// Revision of firmware
    pub firmware_revision: u32,
    /// Handle to the active console input device
    pub console_in_handle: Handle,
    /// Pointer to the interface associated with console_in_handle
    pub con_in: NotImplemented,
    /// Handle to the active console output device
    pub console_out_handle: Handle,
    /// Pointer to the interface associated with console_out_handle
    pub con_out: NotImplemented,
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
