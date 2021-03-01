use core::cell::UnsafeCell;

use uefi::{
    proto::loaded_image::{device_path::DeviceType, DevicePath},
    proto::media::block::BlockIO,
    table::boot::BootServices,
    Handle,
};

use crate::{get_loaded_image, get_system_table};

/// Structure containing EFI disk information
struct EfiDisk<'a> {
    // TODO: add disck device
    /// Handle to disk
    handle: Handle,
    /// Device path
    path: &'a UnsafeCell<DevicePath>,
    /// Block I/O protocol
    block: &'a UnsafeCell<BlockIO>,
    /// Media ID
    media_id: u32,
    /// Whether the device is the boot device
    boot: bool,
    /// Lba of the boot partition
    boot_partition_lba: u64,
}

impl<'a> EfiDisk<'a> {
    /// Get a reference to the efi disk's handle.
    fn handle(&self) -> &Handle {
        &self.handle
    }

    /// Get a reference to the efi disk's path.
    fn path(&self) -> &'a UnsafeCell<DevicePath> {
        &self.path
    }

    /// Get a reference to the efi disk's block.
    fn block(&self) -> &'a UnsafeCell<BlockIO> {
        &self.block
    }

    /// Get a reference to the efi disk's media id.
    fn media_id(&self) -> u32 {
        self.media_id
    }

    /// Get a reference to the efi disk's boot.
    fn is_boot(&self) -> bool {
        self.boot
    }

    /// Get a reference to the efi disk's boot partition lba.
    fn boot_partition_lba(&self) -> u64 {
        self.boot_partition_lba
    }
}

pub struct EfiDiskManager {}

/// Detect and register all disk devices.
pub fn init() {
    use crate::uefi::ResultExt;

    let bt = get_system_table().boot_services();

    // Get a list of all handles supporting the block I/O protocol.
    let handles = bt
        .find_handles::<BlockIO>()
        .expect_success("efi: no block devices available");

    info!("efi: number of block devices: {}", handles.len());

    // EFI gives us both raw devices, and any partitions as child devices. We are only interested in the raw devices, as
    // we handle partition maps internally. We want to pick out the raw devices, and identify the type of these devices.
    //
    // It seems like there should be a better way to identify the type, but raw devices don't appear to get flagged with
    // the type of the device they are: their path nodes are just typed as ATA/SCSI/whatever (except for floppies, which
    // can be identified by their ACPI HID). Child devices do get flagged with a device type.
    //
    // What we do then is make a first pass over all devices to get their block protocol. If a device is a raw device (
    // is_logical_partition() == true), then we do some guesswork:
    //
    // 1. If device path node is ACPI, check HID, mark as floppy if matches.
    // 2. Otherwise, if removable, read-only, and block size if 2048, mark as CD.
    // 3. Otherwise, mark as HD.
    //
    // We then do a pass ovr the child devices, and if they identify the type of the their parent, then that overrides
    // the type guessed for the raw device.
    handles.iter().for_each(|&handle| {
        let block_cell = bt
            .handle_protocol::<BlockIO>(handle)
            .expect("efi: warning: failed to open block I/O")
            .unwrap();

        unsafe {
            let block = &*block_cell.get();
            let media = block.media();

            // Get device path and ignore the end of hardware path
            let device_path_cell = bt
                .handle_protocol::<DevicePath>(handle)
                .expect("efi: failed to retrieve `DevicePath` protocol from block image handler")
                .unwrap();
            let device_path = unsafe { &mut *device_path_cell.get() };
            match device_path.device_type {
                DeviceType::End => return,
                _ => {}
            }

            // TODO: fix the boot device detection logic
            let loaded_image_device = get_loaded_image().device();

            /// create the new disk
            let disk = EfiDisk {
                handle,
                path: device_path_cell,
                block: block_cell,
                media_id: media.media_id(),
                boot: &loaded_image_device as *const _ == &handle as *const _,
                boot_partition_lba: if media.is_media_preset() {
                    media.last_block() + 1
                } else {
                    0
                },
            };

            if disk.is_boot() {
                info!("efi: boot device is: {:p}", disk.path());
            }
        }
    });

    // TODO: implement list below
    // - get handlers
    // - iterate all handlers
    //  - get device path
    //  - open protocol
}
