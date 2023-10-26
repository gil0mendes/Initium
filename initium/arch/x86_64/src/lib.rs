#![no_std]

extern crate log;
extern crate raw_cpuid;
extern crate x86_64;

mod time;

use raw_cpuid::CpuId;

pub struct ArchManager {
    pub time_manager: time::TimeManager,
}

impl ArchManager {
    /// Check if CPUID is supported
    ///
    /// If we can change the EFLAGS.ID, it is.
    fn check_cpuid_support(&self) {
        use x86_64::registers::rflags::{self, RFlags};

        // read the RFlags and toggle the value of the
        // ID flag
        let mut original_flags = rflags::read();
        let old_id_val = original_flags.contains(RFlags::ID);

        original_flags.toggle(RFlags::ID);
        unsafe { rflags::write(original_flags) };

        // Read the flags again to check if it changed
        let new_flags = rflags::read();

        if old_id_val == new_flags.contains(RFlags::ID) {
            panic!("CPUID not supported");
        }
    }

    /// Perform early architecture initialization
    pub fn init(&mut self) {
        self.check_cpuid_support();

        // Initialize the time functions
        self.time_manager.init();
    }

    pub fn new() -> Self {
        ArchManager {
            time_manager: time::TimeManager::new(),
        }
    }
}
