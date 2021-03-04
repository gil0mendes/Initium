/// Types of disk devices
///
/// Primarily used for naming purpose.
pub enum DiskType {
    /// Hard drive/other.
    HD,
    /// CDROM.
    CDROM,
    /// Floppy drive.
    Floppy,
}

/// Structure representing a disk device.
pub struct Disk {
    /// Id of the disk
    id: u8,

    /// Type of the disk
    disk_type: DiskType,
}
