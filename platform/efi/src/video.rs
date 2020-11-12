use uefi::proto::console::gop::{GraphicsOutput};
use uefi::table::boot::BootServices;
use uefi::ResultExt;

use common::video::{VideoManager, VideoMode, ConsoleOut};

/// Manager for EFI video related opetations
pub struct EFIVideoManager<'boot> {
    gop: Option<&'boot mut GraphicsOutput<'boot>>,
}

impl<'boot> EFIVideoManager<'boot> {
    /// Create a new instance of this manager
    pub fn new() -> Self {
        EFIVideoManager {
            gop: Option::None,
        }
    }

    pub fn get_graphics_output(&mut self) -> &mut GraphicsOutput<'boot> {
        self.gop.as_mut().unwrap()
    }

    /// Set a large graphics mode.
    fn set_init_graphics_mode(&mut self) {
        assert!(self.gop.is_some(), "A reference for the Graphics Output protocol is required");

        let gop_ref = self.gop.as_mut().unwrap();

        let mode = gop_ref
            .modes()
            .map(|mode| mode.expect("Warnings encountered while querying mode"))
            .find(|ref mode| {
                let info = mode.info();

                info.resolution() == (1024, 768)
            }).unwrap();

        gop_ref
            .set_mode(&mode)
            .expect_success("Failed to set graphics mode.")
    }

    pub fn init(&mut self, bt: &BootServices) {
        // Look for a graphics output handler
        let mut gop_proto = bt.locate_protocol::<GraphicsOutput>().expect_success("UEFI Graphics Output Protocol is not supported");
        let gop = unsafe { &mut *gop_proto.get() };
        self.gop = Option::Some(gop);

        self.set_init_graphics_mode();
    }
}

impl VideoManager for EFIVideoManager<'boot> {
    fn get_mode(&self) -> VideoMode {
        VideoMode {
            width: 1024,
            height: 768,
        }
    }

    fn get_console_out(&self) -> &dyn ConsoleOut {
        todo!()
    }
}
