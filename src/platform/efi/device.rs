use core::ptr;
use core::usize;

use uefi::proto::device_path::{DevicePath, DeviceType};

pub struct Device {}

/// Get the next device path node in a device path.
pub fn next_device_node(path: &DevicePath) -> Option<DevicePath> {
    let path_ptr = path as *const _ as usize;
    let length = ((path.length[0] as usize) << 8) | path.length[1] as usize;

    let new_ptr = path_ptr + length;
    let next_path = unsafe { ptr::read(new_ptr as *const DevicePath) };

    // FIXME: for some reason when I use a `_` to catch all the values other than None it doesn't work as expected
    match next_path.device_type {
        DeviceType::End => {
            return None;
        }
        DeviceType::Hardware => Some(next_path),
        DeviceType::Acpi => Some(next_path),
        DeviceType::Messaging => Some(next_path),
        DeviceType::Media => Some(next_path),
        DeviceType::BiosBootSpec => Some(next_path),
    }
}

/// get the last device path node in a device path.
pub unsafe fn last_device_node(path: &DevicePath) -> &DevicePath {
    let mut path = path;

    let mut val;

    loop {
        let next = next_device_node(path);

        match next {
            Some(_) => {
                val = next.unwrap();
                path = &val;
            }
            None => {
                break;
            }
        }
    }

    let ptr_addr = path as *const _ as usize;
    return &*(ptr_addr as *const DevicePath);
}
