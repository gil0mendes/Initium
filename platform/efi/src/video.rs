use uefi::proto::console::gop::{GraphicsOutput};
use uefi::table::boot::BootServices;
use uefi_exts::BootServicesExt;

pub struct VideoManager {
}

impl VideoManager {
    pub fn new() -> Self {
        VideoManager {
        }
    }

    /// Set a large graphics mode.
    fn set_init_graphics_mode(&self, gop: &mut GraphicsOutput) {
        let mode = gop
            .modes()
            .find(|ref mode| {
                let info = mode.info();

                info.resolution() == (1024, 768)
            }).unwrap();

        gop
            .set_mode(&mode)
            .expect("Failed to set graphics mode.")
    }

    pub fn init(&mut self, bt: &BootServices) {
        // Look for a graphics output handler
        if let Some(mut gop_proto) = bt.find_protocol::<GraphicsOutput>() {
            let gop = unsafe { gop_proto.as_mut() };
            self.set_init_graphics_mode(gop);
        } else {
            warn!("UEFI Graphics Output Protocol is not supported");
        }
    }
}
