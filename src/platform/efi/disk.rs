use alloc::vec::Vec;
use core::{cell::UnsafeCell, ptr};

use uefi::{
    proto::device_path::{AcpiDevicePath, DevicePath, DeviceType},
    proto::media::block::BlockIO,
    Handle,
};

use crate::disk::{Disk, DiskType};

use super::{
    device::{is_child_device_node, last_device_node},
    get_loaded_image, get_system_table,
};

/// Size of a CD sector
const CD_SECTOR_SIZE: u32 = 2048;

/// floppy HID
const FLOPPY_HUI: u32 = 0x060441d0;

/// Structure containing EFI disk information
struct EfiDisk<'a> {
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
    /// Common disk structure
    disk: Option<Disk>,
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

    /// Get common disk structure
    fn get_common_disk(&self) -> Option<&Disk> {
        self.disk.as_ref()
    }

    /// Set the common disk structure
    fn set_common_disk(&mut self, disk: Option<Disk>) {
        self.disk = disk
    }
}

pub struct EfiDiskManager {}

/// Detect and register all disk devices.
pub fn init() {
    use uefi::ResultExt;

    let mut raw_devices: Vec<EfiDisk> = Vec::new();
    let mut child_devices: Vec<EfiDisk> = Vec::new();

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
            let device_path = &mut *device_path_cell.get();
            match device_path.device_type {
                DeviceType::End => return,
                _ => {}
            }

            // TODO: fix the boot device detection logic
            let loaded_image_device = get_loaded_image().device();

            info!("1>> {:x}", (&loaded_image_device as *const _ as usize));
            let p = 50;
            info!("2>> {:x}", (&p as *const _ as usize));

            // create the new disk
            let mut disk = EfiDisk {
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
                disk: None,
            };

            if disk.is_boot() {
                info!("efi: boot device is: {:p}", disk.path());
            }

            if media.is_logical_partition() {
                child_devices.push(disk);
            } else {
                let disk_path = &(*disk.path().get());
                let last_node = last_device_node(disk_path);

                let mut common_disk = Disk::new();
                if last_node.device_type == DeviceType::Acpi {
                    let acpi_device =
                        ptr::read(last_node as *const DevicePath as *const AcpiDevicePath);

                    // Check EISA ID for a floppy
                    match acpi_device.hid {
                        FLOPPY_HUI => {
                            common_disk.disk_type = DiskType::Floppy;
                        }
                        _ => {}
                    }
                } else if media.is_removable_media()
                    && media.is_read_only()
                    && media.block_size() == CD_SECTOR_SIZE
                {
                    common_disk.disk_type = DiskType::CDROM;
                }

                disk.set_common_disk(Some(common_disk));
                raw_devices.push(disk);
            }
        }
    });

    // pass over child devices to identify their types
    child_devices.iter().for_each(|child| {
        let child_path = unsafe { &(*child.path().get()) };
        let last_path = last_device_node(child_path);

        // identify the parent device
        raw_devices.iter_mut().for_each(|parent| {
            let parent_path = unsafe { &*parent.path().get() };
            let child_path = unsafe { &*child.path().get() };

            // ignore when there is no relation between the child and the possible parent
            if !is_child_device_node(parent_path, child_path) {
                return;
            }

            // Mark the parent as the boot device if the partition is the boot partition
            if child.is_boot() {
                parent.boot = true;
            }

            if last_path.device_type != DeviceType::Media {
                return;
            }

            unsafe {
                let sub_type = *(&last_path.sub_type as *const _ as *const u8);
                info!(">>> {:x}", sub_type);
            }

            // match last_path.sub_type {
            //     uefi::proto::device_path::DeviceSubType::EndInstance => {}
            //     uefi::proto::device_path::DeviceSubType::EndEntire => {}
            // }
        });
    });
}
