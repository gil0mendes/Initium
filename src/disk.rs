/// Holds the last disk count to generate the unique disk identifier.
static mut DISK_COUNT: u8 = 0;

/// Types of disk devices
///
/// Primarily used for naming purpose.
#[derive(Debug)]
pub enum DiskType {
    /// Hard drive/other.
    HD,
    /// CDROM.
    CDROM,
    /// Floppy drive.
    Floppy,
}

/// Structure representing a disk device.
#[derive(Debug)]
pub struct Disk {
    /// Id of the disk
    pub id: u8,

    /// Type of the disk
    pub disk_type: DiskType,
}

impl Disk {
    /// Generate a new common disk structure
    pub fn new() -> Self {
        let new_disk_id = unsafe {
            DISK_COUNT = DISK_COUNT + 1;
            DISK_COUNT
        };

        Self {
            id: new_disk_id,
            disk_type: DiskType::HD,
        }
    }
}
