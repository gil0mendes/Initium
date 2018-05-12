/// Type of Allocation
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u32)]
pub enum AllocateType {
    /// allocate any available range of pages that satisfies the request.
    AnyPages,
    /// allocate any available range of pages whose uppermost address is less than or equal to
    /// the address pointed to by Memory on input
    MaxAddress,
    /// allocate pages at the address pointed to by Memory on input.
    Address,
}

/// Type of memory
// MemoryType is a u32; a C-style enum repr[(u32)] is insufficient as values
// outside those defined explicitly in the spec are allowed.
// Types from 0x70000000 through 0x7FFFFFFF are reserved for OEM use.
// Types from 0x80000000 through 0xFFFFFFFF are resreved for UEFI OS loaders.
pub type MemoryType = u32;

/// Memory that is not usable.
pub const MEMORY_TYPE_RESERVED: MemoryType = 0;

/// The code portions of a UEFI application. After exiting boot services, this
/// memory may be reclaimed when no longer in use
pub const MEMORY_TYPE_LOADER_CODE: MemoryType = 1;

/// The data portions of a UEFI application, and the default for allocating pool
/// memory. After exiting boot services, this memory may be reclaimed when longer
/// in use.
pub const MEMORY_TYPE_LOADER_DATA: MemoryType = 2;

/// The code portions of a loaded UEFI Boot Services Driver. After exiting boot
/// services, this memory may be reclaimed.
pub const MEMORY_TYPE_BOOT_SERVICES_CODE: MemoryType = 3;

/// The data portions of a loaded UEFI Boot Services Driver. After exiting boot
/// services, this memory may be reclaimed.
pub const MEMORY_TYPE_BOOT_SERVICES_DATA: MemoryType = 4;

/// The code portions of a loaded UEFI Runtime driver. This memory is to be
/// preserved by the bootloader/OS in the working and ACPI S1-S3 states.
pub const MEMORY_TYPE_RUNTIME_SERVICES_CODE: MemoryType = 5;

/// The data portions of a loaded UEFI Runtime driver. This memory is to be
/// preserved by the bootloader/OS in the working and ACPI S1-S3 states.
pub const MEMORY_TYPE_RUNTIME_SERVICES_DATA: MemoryType = 6;

/// Free (unallocated) memory.
pub const MEMORY_TYPE_CONVENTIONAL_MEMORY: MemoryType = 7;

/// Memory in which errors have been detected. Not to be used.
pub const MEMORY_TYPE_UNUSABLE_MEMORY: MemoryType = 8;

/// Memory that holds the ACPI tables. This memory is to be preserved by the
/// bootloader/OS until ACPI is enabled, and then may be reclaimed.
pub const MEMORY_TYPE_ACPI_RECLAIM_MEMORY: MemoryType = 9;

/// Address space reserved for use by the firmware. This memory is to be
/// preserved by the bootloader/OS in the working and ACPI S1-S3 states.
pub const MEMORY_TYPE_ACPI_MEMORY_NVS: MemoryType = 10;

/// Used by system firmware to request that a memory-mapped IO region be mapped
/// by the OS to a virtual address to it can be accessed by EFI runtime services.
/// After exiting boot services, The OS should not use this memory.
pub const MEMORY_TYPE_MEMORY_MAPPED_IO: MemoryType = 11;

/// System memory-mapped IO region that is used to translate memory cycles to
/// IO cycles by the processor. After exiting boot services, the OS should not use
/// this memory.
pub const MEMORY_TYPE_MEMORY_MAPPED_IO_PORT_SPACE: MemoryType = 12;

/// Address space reserved by the firmware for code that is part of the processor.
/// This memory is to be preserved by the bootloader/OS in the working and ACPI
/// S1-S3 states.
pub const MEMORY_TYPE_PAL_CODE: MemoryType = 13;

/// A memory region that operates as EFI conventional memory, and also supports
/// byte-addressable non-volatility.
pub const MEMORY_TYPE_PERSISTENT_MEMORY: MemoryType = 14;

pub type PhysicalAddress = u64;

pub type VirtualAddress = u64;

bitflags!{
    pub struct MemoryAttributeFlags: u64 {
        /// Memory cacheability attribute: The memory region
        /// supports being configured as not cacheable.
        const MEMORY_UC = 0x1;

        /// Memory cacheability attribute: The memory region
        /// supports being configured as write combining.
        const MEMORY_WC = 0x2;

        /// Memory cacheability attribute: The memory region
        /// supports being configured as cacheable with a “write
        /// through” policy. Writes that hit in the cache will also be
        /// written to main memory.
        const MEMORY_WT = 0x4;

        /// Memory cacheability attribute: The memory region
        /// supports being configured as cacheable with a “write
        /// back” policy. Reads and writes that hit in the cache do not
        /// propagate to main memory. Dirty data is written back to
        /// main memory when a new cache line is allocated.
        const MEMORY_WB = 0x8;

        /// Memory cacheability attribute: The memory region
        /// supports being configured as not cacheable, exported,
        /// and supports the “fetch and add” semaphore mechanism.
        const MEMORY_UCE = 0x10;

        /// Physical memory protection attribute: The memory region
        /// supports being configured as write-protected by system
        /// hardware. This is typically used as a cacheability attribute
        /// today. The memory region supports being configured as
        /// cacheable with a "write protected" policy. Reads come
        /// from cache lines when possible, and read misses cause
        /// cache fills. Writes are propagated to the system bus and
        /// cause corresponding cache lines on all processors on the
        /// bus to be invalidated.
        const MEMORY_WP = 0x1000;

        /// Physical memory protection attribute: The memory region
        /// supports being configured as read-protected by system
        /// hardware.
        const MEMORY_RP = 0x2000;

        /// Physical memory protection attribute: The memory region
        /// supports being configured so it is protected by system
        /// hardware from executing code.
        const MEMORY_XP = 0x4000;

        /// Runtime memory attribute: The memory region refers to
        /// persistent memory
        const MEMORY_NV = 0x8000;

        /// The memory region provides higher reliability relative to
        /// other memory in the system. If all memory has the same
        /// reliability, then this bit is not used.
        const MEMORY_MORE_RELIABLE = 0x10000;

        /// Physical memory protection attribute: The memory region
        /// supports making this memory range read-only by system
        /// hardware.
        const MEMORY_RO = 0x20000;

        /// Runtime memory attribute: The memory region needs to
        /// be given a virtual mapping by the operating system when
        /// SetVirtualAddressMap() is called (described in
        /// Section 8.4 of the spec).
        const MEMORY_RUNTIME = 0x8000000000000000;
    }
}

#[derive(Debug, Clone)]
pub struct MemoryDescriptor {
    /// The type of the memory region
    pub type_: MemoryType, // EFI_MEMORY_TYPE

    /// Physical start of region. 4K aligned. Maximum 0xfffffffffffff000
    pub physical_start: PhysicalAddress,

    /// Virtual start of region. 4K aligned. Maximum 0xfffffffffffff000
    pub virtual_start: VirtualAddress,

    /// Number of 4K pages in region
    pub number_of_pages: u64,

    /// Attributes of the memory region
    pub attribute: MemoryAttributeFlags
}
