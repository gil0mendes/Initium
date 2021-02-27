use platform::target_device_probe;

/// Initialize the device manager
pub fn init() {
    target_device_probe();

    // TODO: print device list
}
