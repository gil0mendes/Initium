//! Video mode management

use alloc::boxed::Box;
use common::video::{VideoModeInfo, VideoModeOps};

use crate::console::console_manager;

pub struct VideoManager {
    current_video_mode: Option<Box<dyn VideoModeOps>>,
}

impl VideoManager {
    /// Get the current video mode set
    pub fn get_mode_info(&self) -> Option<VideoModeInfo> {
        match &self.current_video_mode {
            Some(mode) => Some(mode.get_mode_info()),
            None => None,
        }
    }

    pub fn set_mode(&mut self, mut mode: Option<Box<dyn VideoModeOps>>, set_console: bool) {
        let console_manager = console_manager();

        // when there was an already set console we need to de-initialize the console
        match console_manager.primary_console {
            None => {}
            Some(ref mut console) => {
                console.deinit();
            }
        };

        console_manager.set_primary_console(None);

        if mode.is_some() {
            mode.as_mut().unwrap().set_mode();
        }

        self.current_video_mode = mode;

        if !set_console {
            return;
        }

        let console = self.current_video_mode.as_ref().unwrap().create_console();
        if console.is_some() {
            console_manager.set_primary_console(console);
            console_manager.primary_console.as_mut().unwrap().init();
        }
    }
}

static mut VIDEO_MANAGER: VideoManager = VideoManager {
    current_video_mode: None,
};

/// Get video manager
pub fn get_video_manager() -> &'static mut VideoManager {
    unsafe { &mut VIDEO_MANAGER }
}
