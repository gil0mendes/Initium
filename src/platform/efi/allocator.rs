use alloc::alloc::{GlobalAlloc, Layout};
use core::ptr::{self, NonNull};

use uefi::table::boot::{BootServices, MemoryType};

/// Reference to the boot services table, used to call the pool memory allocation functions.
///
/// The inner pointer is only safe to dereference if UEFI boot services have not been
/// exited by the host application yet.
static mut BOOT_SERVICES: Option<NonNull<BootServices>> = None;

/// Initializes the allocator.
///
/// # Safety
///
/// This function is unsafe because you _must_ make sure that exit_boot_services
/// will be called when UEFI boot services will be exited.
pub unsafe fn init(boot_services: &BootServices) {
    BOOT_SERVICES = NonNull::new(boot_services as *const _ as *mut _);
}

/// Access the boot services
pub fn boot_services() -> NonNull<BootServices> {
    unsafe { BOOT_SERVICES.expect("Boot services are unavailable or have been exited") }
}

/// Notify the allocator library that boot services are not safe to call anymore
///
/// You must arrange for this function to be called on exit from UEFI boot services
pub fn exit_boot_services() {
    unsafe {
        BOOT_SERVICES = None;
    }
}

/// Allocator which uses the UEFI pool allocation functions.
///
/// Only valid for as long as the UEFI boot services are available.
///
/// This allocation is only used while the bootloader is running. Before exiting from the firmware into the Kernel, a
/// memory map is created to tell the Kernel the memory layout and reduce the allocation complexity on the kernel side.
pub struct Allocator;

#[allow(clippy::cast_ptr_alignment)]
unsafe impl GlobalAlloc for Allocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let mem_type = MemoryType::LOADER_DATA;
        let size = layout.size();
        let align = layout.align();

        if align > 8 {
            // allocate more space for alignment
            let ptr = if let Ok(ptr) = boot_services()
                .as_ref()
                .allocate_pool(mem_type, size + align)
            {
                ptr
            } else {
                return ptr::null_mut();
            };

            // calculate align offset
            let mut offset = ptr.align_offset(align);
            if offset == 0 {
                offset = align;
            }

            let return_ptr = ptr.add(offset);

            // store allocated pointer before the struct
            (return_ptr.cast::<*mut u8>()).sub(1).write(ptr);
            return_ptr
        } else {
            boot_services()
                .as_ref()
                .allocate_pool(mem_type, size)
                .unwrap_or(ptr::null_mut())
        }
    }

    unsafe fn dealloc(&self, mut ptr: *mut u8, layout: Layout) {
        if layout.align() > 8 {
            ptr = (ptr.cast::<*mut u8>()).sub(1).read();
        }

        boot_services().as_ref().free_pool(ptr).unwrap();
    }
}

#[alloc_error_handler]
fn out_of_memory(layout: ::core::alloc::Layout) -> ! {
    panic!(
        "Ran out of free memory while trying to allocate {:#?}",
        layout
    );
}

#[global_allocator]
static ALLOCATOR: Allocator = Allocator;
