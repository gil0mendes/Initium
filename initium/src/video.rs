//! Video mode management

use common::video::VideoMode;

pub struct VideoManager {
    current_video_mode: Option<VideoMode>,
}

impl VideoManager {
    /// Get the current video mode set
    pub fn get_mode(&self) -> Option<VideoMode> {
        self.current_video_mode
    }

    pub fn set_mode(&mut self, mode: VideoMode, set_console: bool) {
        self.current_video_mode = Some(mode);

        // TODO: check if we need to initialize the video device
    }
}

pub static VIDEO_MANAGER: VideoManager = VideoManager {
    current_video_mode: None,
};
