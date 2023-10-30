use core::usize;
use core::{cmp::min, ptr};

use alloc::boxed::Box;
use alloc::string::String;
use log::info;
use rlibc::memcmp;
use uefi::prelude::BootServices;
use uefi::proto::device_path::text::AllowShortcuts;
use uefi::proto::device_path::{DevicePath, DeviceType};
use uefi::{CString16, Handle};

/// Open the device path protocol
pub fn get_device_path(bs: &BootServices, handle: Handle) -> Option<Box<DevicePath>> {
    let result = bs.open_protocol_exclusive::<DevicePath>(handle);

    let path = match result {
        Ok(p) => p,
        Err(_) => return None,
    };

    Some(path.to_boxed())
}

// pub struct Device {}

// /// Get the next device path node in a device path.
// pub fn next_device_node(path: &DevicePath) -> Option<DevicePath> {
//     let path_ptr = path as *const _ as usize;

//     let new_ptr = path_ptr + path.size();
//     let next_path = unsafe { ptr::read(new_ptr as *const DevicePath) };

//     // FIXME: for some reason when I use a `_` to catch all the values other than None it doesn't work as expected
//     match next_path.device_type {
//         DeviceType::End => {
//             return None;
//         }
//         DeviceType::Hardware => Some(next_path),
//         DeviceType::Acpi => Some(next_path),
//         DeviceType::Messaging => Some(next_path),
//         DeviceType::Media => Some(next_path),
//         DeviceType::BiosBootSpec => Some(next_path),
//     }
// }

// /// get the last device path node in a device path.
// pub fn last_device_node(path: &DevicePath) -> &DevicePath {
//     let mut path = path;

//     let mut val;

//     loop {
//         let next = next_device_node(path);

//         match next {
//             Some(_) => {
//                 val = next.unwrap();
//                 path = &val;
//             }
//             None => {
//                 break;
//             }
//         }
//     }

//     let ptr_addr = path as *const _ as usize;
//     return unsafe { &*(ptr_addr as *const DevicePath) };
// }

// /// Determine if a device path is child of another.
// pub fn is_child_device_node(parent: &DevicePath, child: &DevicePath) -> bool {
//     let mut parent = parent;
//     let mut child = child;

//     let mut temp_parent;
//     let mut temp_child;

//     loop {
//         let child_ptr = child as *const _ as *const u8;
//         let parent_ptr = parent as *const _ as *const u8;

//         unsafe {
//             if memcmp(child_ptr, parent_ptr, min(parent.size(), child.size())) != 0 {
//                 return false;
//             }
//         }

//         let parent_opt = next_device_node(parent);
//         let child_opt = next_device_node(child);

//         if parent_opt.is_none() {
//             return child_opt.is_some();
//         }

//         temp_parent = parent_opt.unwrap();
//         temp_child = child_opt.unwrap();

//         parent = &temp_parent;
//         child = &temp_child;
//     }
// }
