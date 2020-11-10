#![no_std]

pub mod video;

pub struct PlatformManager<'a> {
    pub video_manager: &'a dyn video::VideoManager,
}
