//! EFI memory allocation manager
//!
//! On EFI, we don't use the generic memory manage code. This is because, while we're still in boot service mode, the
//! firmware is in control of the memory map and we should use memory allocation services to allocate memory. Since it
//! is possible for the memory map to change underneath us, we cannot just get the memory map once during init and use
//! it with the generic MM code.

use alloc::vec;
use log::info;
use uefi::table::boot::{BootServices, MemoryType};

use crate::alloc::vec::Vec;
use uefi::ResultExt;

// EFI specifies page size as 4KB regardless of the system.
const EFI_PAGE_SIZE: u64 = 0x1000;

#[derive(Debug)]
pub struct MemoryManager {}

impl MemoryManager {
    pub fn new() -> Self {
        MemoryManager {}
    }

    pub fn init(&self, bt: &BootServices) {
        // Get the estimated map size
        let mem_map_size = bt.memory_map_size();
        let buffer_size = mem_map_size.map_size + 2 * mem_map_size.entry_size;

        // Build a buffer bigger enough to to handle the memory map
        let mut buffer = vec![0_u8; buffer_size];

        let memory_map = bt
            .memory_map(&mut buffer)
            .expect("EFI: failed to retrieve UEFI memory map");
        let desc_iter = memory_map.entries();

        assert!(desc_iter.len() > 0, "Memory map is empty");

        // For information purposes, we print out a list of all the usable memory we see in the memory map. Don't print
        // out everything, the memory map is probably pretty big (e.g. OVMF under QEMU returns a map with nearly 50
        // entries here).
        info!("efi: usable memory ranges ({} total)", desc_iter.len());
        for (j, descriptor) in desc_iter.enumerate() {
            match descriptor.ty {
                MemoryType::CONVENTIONAL => {
                    let size = descriptor.page_count * EFI_PAGE_SIZE;
                    let end_address = descriptor.phys_start + size;
                    info!(
                        "> {:#x} - {:#x} ({} KiB)",
                        descriptor.phys_start, end_address, size
                    );
                }
                _ => {}
            }
        }
    }
}
