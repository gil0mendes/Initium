pub mod types;
pub use self::types::*;

use core::mem::size_of;
use types::*;

pub type AllocatePages = unsafe extern "win64" fn (
    type_: AllocateType,
    memory_type: MemoryType,
    pages: usize,
    memory: *mut PhysicalAddress) -> Status;

pub type FreePages = unsafe extern "win64" fn(
    memory: PhysicalAddress,
    pages: usize) -> Status;

/// Returns the current boot services memory map and memory map key
pub type GetMemoryMap = unsafe extern "win64" fn(
    // The size of `memory_map` in bytes. On failure, firmware will write the size the
    // map needs to be here, so you can try again.
    memory_map_size: *mut usize,
    // An array of MemoryDescriptors, to contain the output
    memory_map: *mut MemoryDescriptor,
    // A key for the current memory map
    map_key: *mut usize,
    // The size of an individual memory descriptor
    descriptor_size: *mut usize,
    descriptor_version: *mut u32) -> Status;

// Allocates a pool of a particular type
pub type AllocatePool = unsafe extern "win64" fn(
    // The type of pool to allocate.
    pool_type: MemoryType,
    // The number of bytes to allocate from the pool.
    size: usize,
    // A pointer to a pointer to the allocated buffer if the call succeeds; undefined otherwise.
    buffer: *mut *mut () ) -> Status;

// Frees allocated pool
pub type FreePool = unsafe extern "win64" fn(
    // pointer to the buffer to free
    buffer: *mut () ) -> Status;

use boot_services::BootServices;

/// Allocates pages of a particular type.
///
/// `boot_services`: A reference to the Boot Services structure
///
/// `allocation_type`: The type of allocation to perform
///
/// `memory_type`: The type of memory to allocate
///
/// `pages`: The number of contiguous 4K pages to allocate
///
/// `memory`: An optional PhysicalAddress, required if `allocation_type` is `MaxAddress`
///           or `Address`
pub fn allocate_pages(
    boot_services: &BootServices,
    allocation_type: AllocateType,
    memory_type: MemoryType,
    pages: usize,
    memory: Option<PhysicalAddress>)
    -> Result<PhysicalAddress, Status>
{
    assert!(allocation_type==AllocateType::AnyPages || memory.is_some());
    let mut addr: PhysicalAddress = match memory {
        Some(pa) => pa,
        None => unsafe { ::core::mem::uninitialized() }
    };
    unsafe {
        match (boot_services.allocate_pages)(allocation_type, memory_type, pages, &mut addr) {
            STATUS_SUCCESS => Ok(addr),
            status => Err(status),
        }
    }
}

/// Frees allocated pages.
///
/// `boot_services`: A reference to the Boot Services structure
///
/// `memory`: The base physical address of the pages to be freed
///
/// `pages`: The number of contiguous 4K pages to free
pub fn free_pages(
    boot_services: &BootServices,
    memory: PhysicalAddress,
    pages: usize) -> Result<(), Status>
{
    unsafe {
        match (boot_services.free_pages)(memory, pages) {
            STATUS_SUCCESS => Ok(()),
            status => Err(status),
        }
    }
}

/// A successful result from get_memory_map()
pub struct MemoryMap<'a> {
    /// The memory map
    pub map: &'a mut [MemoryDescriptor],
    /// The map_key, which may be required later for other UEFI calls.
    pub map_key: usize,
    /// The size, in bytes, of an individual `MemoryDescriptor` (returned by the firmware,
    /// but should not differ from the size of the type in this crate, unless a new UEFI
    /// version has it different)
    pub descriptor_size: usize,
    /// The version number associated with the `MemoryDescriptor` type.
    pub descriptor_version: u32,
}

/// An unsuccessful result from get_memory_map()
pub struct MemoryMapFailure {
    /// The return status
    pub status: Status,
    /// If status was `STATUS_BUFFER_TOO_SMALL`, this will be Some(size) in bytes of memory
    /// that is required, so that you can try again with a bigger memory buffer.
    pub size_required: Option<usize>,
}

/// Either fills the memory with MemoryDescriptors and returns some MemoryMap, or else
/// it fails and returns a MemoryMapFailure structure with details.
pub fn get_memory_map<'a>(
    // A reference to the Boot Services structure
    boot_services: &BootServices,
    // Memory that
    memory: &'a mut [MemoryDescriptor])
    -> Result<MemoryMap<'a>, MemoryMapFailure>
{
    let mut len = memory.len() * size_of::<MemoryDescriptor>();
    let mut map: MemoryMap = unsafe { ::core::mem::uninitialized() };

    unsafe {
        match (boot_services.get_memory_map)(&mut len,
                                             memory.as_mut_ptr(),
                                             &mut map.map_key,
                                             &mut map.descriptor_size,
                                             &mut map.descriptor_version)
        {
            STATUS_SUCCESS => {
                let index = len / size_of::<MemoryDescriptor>();
                let (filled, _empty) =  memory.split_at_mut(index);
                map.map = filled;
                Ok(map)
            },
            STATUS_BUFFER_TOO_SMALL => Err(MemoryMapFailure {
                status: STATUS_BUFFER_TOO_SMALL,
                size_required: Some(len),
            }),
            status => Err(MemoryMapFailure {
                status: status,
                size_required: None,
            }),
        }
    }
}
