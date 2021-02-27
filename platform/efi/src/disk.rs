use uefi::{proto::media::block::BlockIO, table::boot::BootServices};

use crate::get_system_table;

pub struct DiskManager {}

/// Detect and register all disk devices.
pub fn init() {
    use crate::uefi::ResultExt;

    let bt = get_system_table().boot_services();

    // Get a list of all handles supporting the block I/O protocol.
    let handles = bt
        .find_handles::<BlockIO>()
        .expect_success("efi: no block devices available");

    info!("efi: number of block devices {}", handles.len());

    handles.iter().for_each(|&handle| {
        let block = bt
            .handle_protocol::<BlockIO>(handle)
            .expect("efi: warning: failed to open block I/O")
            .unwrap();

        unsafe {
            let block = &*block.get();
            let media = block.media();
            info!(
                "efi: device id {} is removable {}",
                &media.media_id(),
                media.is_removable_media()
            );
        }
    });

    // TODO: implement list below
    // - get handlers
    // - iterate all handlers
    //  - get device path
    //  - open protocol
}
