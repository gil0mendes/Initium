use alloc::string::String;

use crate::{platform::target_device_probe, println};

/// Types of devices.
enum DeviceType {
    /// Local disk.
    Disk,
    /// Virtual device (e.g. boot image)
    Virtual,
}

/// Base device structure.
struct Device {
    /// Name of the device.
    name: String,
    /// Type of the device.
    device_type: DeviceType,
}

/// Initialize the device manager
pub fn init() {
    target_device_probe();

    // TODO: print device list
}
