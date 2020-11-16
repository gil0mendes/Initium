use common::video::{VideoManager, ConsoleOut};
use crate::console::{ConsoleOutManager, CONSOLE_MANAGER};
use core::ptr::NonNull;
use crate::video::{VIDEO_MANAGER, EFIVideoManager};
use common::PlatformManager;

static mut PLATFORM_MANAGER: Option<EFIPlatformManager> = None;

/// Struct to store all global pointers that we need to manage platform state
pub struct EFIPlatformManager {}

impl common::PlatformManager for EFIPlatformManager {
    fn video_manager(&self) -> NonNull<dyn VideoManager> {
        unsafe {
            let manager = VIDEO_MANAGER.as_ref().expect("The video manager is not available.");
            NonNull::new(manager as *const _ as *mut EFIVideoManager).unwrap()
        }
    }

    fn console_manager(&self) -> NonNull<dyn ConsoleOut> {
        unsafe {
            let manager = CONSOLE_MANAGER.as_ref().expect("The console manager is not available");
            NonNull::new(manager as *const _ as *mut ConsoleOutManager).unwrap()
        }
    }
}

/// Initialize the platform manager
///
/// This must be called as early as possible before trying to use all of the platform managers.
pub fn init() {
    unsafe {
        if PLATFORM_MANAGER.is_some() {
            return;
        }

        PLATFORM_MANAGER = Some(EFIPlatformManager {});
    }
}

/// Obtain a pointer to platform manager
pub fn platform_manager() -> NonNull<PlatformManager> {
    unsafe {
        let table_ref = PLATFORM_MANAGER
            .as_ref()
            .expect("The platform manager is not available");
        NonNull::new(table_ref as *const _ as *mut EFIPlatformManager).unwrap()
    }
}
