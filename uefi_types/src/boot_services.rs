use types::*;
use memory_services::{AllocatePages, FreePages, GetMemoryMap, AllocatePool, FreePool};
use miscellaneous::{SetWatchdogTimer, Stall, CopyMem};

// #define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42

pub struct BootServices {
    /// Table header
    pub hdr: TableHeader,

    /// Task Priority Service
    /// Raises the task priority level
    pub raise_tpl: NotImplemented,

    /// Task Priority Service
    /// Restores/lowers the task priority level
    pub restore_tpl: NotImplemented,

    /// Memory Service
    /// Allocates pages of a particular type
    pub allocate_pages: AllocatePages,

    /// Memory Service
    /// Frees allocated pages
    pub free_pages: FreePages,

    /// Memory Service
    /// Returns the current boot services memory map and memory map key
    pub get_memory_map: GetMemoryMap,

    /// Memory Service
    /// Allocates a pool of a particular type
    pub allocate_pool: AllocatePool,

    /// Memory Service
    /// Frees allocated pool
    pub free_pool: FreePool,

    /// Event & Timer Service
    /// Creates a general-purpose event structure
    pub create_event: NotImplemented,

    /// Event & Timer Service
    /// Sets an event to be signaled at a particular time
    pub set_timer: NotImplemented,

    /// Event & Timer Service
    /// Stops execution until an event is signaled
    pub wait_for_event: NotImplemented,

    /// Event & Timer Service
    /// Signals an event
    pub signal_event: NotImplemented,

    /// Event & Timer Service
    /// Closes and frees an event structure
    pub close_event: NotImplemented,

    /// Event & Timer Service
    /// Checks whether an event is in the signaled state
    pub check_event: NotImplemented,

    /// Protocol Handler Service
    /// Installs a protocol interface on a device handle
    pub install_protocol_interface: NotImplemented,

    /// Protocol Handler Service
    /// Reinstalls a protocol interface on a device handle
    pub reinstall_protocol_interface: NotImplemented,

    /// Protocol Handler Service
    /// Removes a protocol interface from a device handle
    pub uninstall_protocol_interface: NotImplemented,

    /// Protocol Handler Service
    /// Queries a handle to determine if it supports a specified protocol
    pub handle_protocol: NotImplemented,

    /// Reserved (will be NULL)
    _reserved: *const (),

    /// Protocol Handler Service
    /// Registers an event that is to be signaled whenver an interface
    /// is installed for a specified protocol
    pub register_protocol_notify: NotImplemented,

    /// Protocol Handler Service
    /// Returns an array of handles that support a specified protocol
    pub locate_handle: NotImplemented,

    /// Protocol Handler Service
    /// Locates all devices on a device path that support a specified protocol
    /// and returns the handle to the device that is closest to the path
    pub locate_device_path: NotImplemented,

    /// Protocol Handler Service
    /// Adds, updates, or removes a configuration table from the EFI System Table
    pub install_configuration_table: NotImplemented,

    /// Image Service
    /// Loads an EFI image into memory
    pub image_load: NotImplemented,

    /// Image Service
    /// Transfers control to a loaded image's entry point
    pub image_start: NotImplemented,

    /// Image Service
    /// Exits the image's entry point
    pub exit: NotImplemented,

    /// Image Service
    /// Unloads an image
    pub image_unload: NotImplemented,

    /// Image Service
    /// Terminates boot services
    pub exit_boot_services: NotImplemented,

    /// Miscellaneous Service
    /// Returns a monotonically increasing count for the platform.
    pub get_next_monotonic_count: NotImplemented,

    /// Miscellaneous Service
    /// Stalls the processor
    pub stall: Stall,

    /// Miscellaneous Service
    /// Resets and sets a watchdog timer used during boot services time
    pub set_watchdog_timer: SetWatchdogTimer,

    /// Open and Close Protocol Service
    /// Uses a set of precedence rules to find the gbest set of drivers to
    /// manage a controller
    pub connect_controller: NotImplemented,

    /// Open and Close Protocol Service
    /// Informs a set of drivers to stop managing a controller.
    pub disconnect_controller: NotImplemented,

    /// Open and Close Protocol Service
    /// Adds elements to the list of agents consuming a protocol interface.
    pub open_protocol: NotImplemented,

    /// Open and Close Protocol Service
    /// Removes elements from the list of agents consuming a protocol interface.
    pub close_protocol: NotImplemented,

    /// Open and Close Protocol Service
    /// Retrieve the list of agents that are currently consuming a protocol
    /// interface
    pub open_protocol_information: NotImplemented,

    /// Library Service
    /// Retrieves the list of protocols installed on a handle. The return buffer
    /// is automatically allocated.
    pub protocols_per_handle: NotImplemented,

    /// Library Service
    /// Retrives the list of handles from the handle database that meet the search
    /// criteria. Ther eturn buffer is automatically allocated.
    pub locate_handle_buffer: NotImplemented,

    /// Library Service
    /// Finds the first handle in the handle database that supports the
    /// requested protocol
    pub locate_protocol: NotImplemented,

    /// Library Service
    /// Installs one or more protocol interfaces onto a handle
    pub install_multiple_protocol_interfaces: NotImplemented,

    /// Library Service
    /// Uninstalls one or more protocol interfaces from a handle
    pub uninstall_multiple_protocol_interfaces: NotImplemented,

    /// 32-bit CRC Service
    /// Computes and returns a 32-bit CRC for a data buffer
    pub calculate_crc32: NotImplemented,

    /// Miscellaneous Service
    /// Copies the contents of one buffer to another buffer
    pub copy_mem: CopyMem,

    /// Miscellaneous Service
    /// Fills a buffer with a specified value
    pub set_mem: NotImplemented,

    /// Miscellaneous Service
    /// Creates an event structure as part of an event group
    pub create_event_ex: NotImplemented,
}
