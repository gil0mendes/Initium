use uefi::proto::console::gop::{GraphicsOutput};
use uefi::table::boot::BootServices;
use uefi::ResultExt;

use common::video::{VideoManager, VideoMode};

pub struct EFIVideoManager {
}

impl EFIVideoManager {
    pub fn new() -> Self {
        EFIVideoManager {
        }
    }

    /// Set a large graphics mode.
    fn set_init_graphics_mode(&self, gop: &mut GraphicsOutput) {
        let mode = gop
            .modes()
            .map(|mode| mode.expect("Warnings encountered while querying mode"))
            .find(|ref mode| {
                let info = mode.info();

                info.resolution() == (1024, 768)
            }).unwrap();

        gop
            .set_mode(&mode)
            .expect_success("Failed to set graphics mode.")
    }

    pub fn init(&mut self, bt: &BootServices) {
        // Look for a graphics output handler
        let mut gop_proto = bt.locate_protocol::<GraphicsOutput>().expect_success("UEFI Graphics Output Protocol is not supported");
        let gop = unsafe { &mut *gop_proto.get() };
        self.set_init_graphics_mode(gop);
    }
}

impl VideoManager for EFIVideoManager {
    fn get_mode(&self) -> VideoMode {
        VideoMode {
            width: 1024,
            height: 768,
        }
    }
}
