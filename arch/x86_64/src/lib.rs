#![no_std]
#![no_main]

#![feature(asm)]

#[macro_use]
extern crate log;
extern crate x86_64;
extern crate raw_cpuid;

mod time;

use raw_cpuid::CpuId;

/// Check if CPUID is supported
///
/// If we can change the EFLAGS.ID, it is.
fn check_cpuid_support() {
    use x86_64::registers::rflags::{self, RFlags};

    // read the RFlags and toggle the value of the
    // ID flag
    let mut original_flags = rflags::read();
    let old_id_val = original_flags.contains(RFlags::ID);

    original_flags.toggle(RFlags::ID);
    rflags::write(original_flags);

    // Read the flags again to check if it changed
    let new_flags = rflags::read();

    if old_id_val == new_flags.contains(RFlags::ID) {
        panic!("CPUID not supported");
    }
}

/// Perform early architecture initialization
pub fn arch_init() {
    check_cpuid_support();

    // Initialize the time functions
    let mut time_manager = time::TimeManager::new();
    time_manager.init();

    info!("All Good ;)");
}
