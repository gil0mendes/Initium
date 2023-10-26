///! x86 timing functions.

use raw_cpuid::CpuId;
use x86_64::instructions::port::Port;
use core::arch::x86_64::_rdtsc;

/// Frequency of the PIT
const PIT_FREQUENCY: u64 = 1193182;

/// PIT port definition
const PIT_PORT_MODE: u16 = 0x43;
const PIT_PORT_CHANNEL_0: u16 = 0x40;

/// PIT mode bit definitions
const PIT_MODE_CHANNEL_0: u8 = 0 << 6;
const PIT_MODE_RATE_GENERATOR: u8 = 2 << 1;
const PIT_MODE_ACCESS_LATCH: u8 = 0 << 4;
const PIT_MODE_ACCESS_BOTH: u8 = 3 << 4;

/// Type used to store a time value in milliseconds.
pub type MsTime = u64;

/// This a manager for all time functions
pub struct TimeManager {
    /// TSC cycles per milliseconds
    tsc_cycles_per_msec: u64,

    /// Initial TSC start time
    tsc_start_time: u64,
}

impl TimeManager {
    /// Create a new time manger instance.
    pub fn new() -> Self {
        TimeManager {
            tsc_cycles_per_msec: 0,
            tsc_start_time: 0,
        }
    }

    /// Get the number of cycles per second
    pub fn get_cycles_per_msec(&self) -> u64 {
        self.tsc_cycles_per_msec
    }

    /// Get the current internal time.
    pub fn current_time(&self) -> MsTime {
        let current = unsafe { _rdtsc() };
        (current - self.get_cycles_per_msec()) / self.tsc_cycles_per_msec
    }

    /// Initialize the TSC
    pub fn init(&mut self) {
        let cpuid = CpuId::new();

        // Check TSC support
        {
            let has_tsc = match cpuid.get_feature_info(){
                Some(finfo) => finfo.has_tsc(),
                None => false,
            };

            if !has_tsc {
                panic!("CPU does not support TSC");
            }
        }

        // Build all needed ports
        let mut pit_mode_port: Port<u8> = Port::new(PIT_PORT_MODE);
        let mut pit_channel_0_port: Port<u8> = Port::new(PIT_PORT_CHANNEL_0);

        // Calculate the TSC frequency.
        // First set the PIT to rate generator mode.
        unsafe {
            pit_mode_port.write(PIT_MODE_CHANNEL_0 | PIT_MODE_RATE_GENERATOR | PIT_MODE_ACCESS_BOTH);
            pit_channel_0_port.write(0xff);
            pit_channel_0_port.write(0xff);
        }

        // Wait for the cycle to being
        let mut start_low: u8;
        let mut start_high: u8;

        loop {
            unsafe {
                pit_mode_port.write(PIT_MODE_CHANNEL_0 | PIT_MODE_ACCESS_LATCH);
                start_low = pit_channel_0_port.read();
                start_high = pit_channel_0_port.read();
            }

            if start_high == 0xff {
                break;
            }
        }

        // get the start TSC value
        let mut tsc_start_time;
        unsafe {
            tsc_start_time = _rdtsc();
        }

        // Wait for the high byte to drop to 128
        let mut end_low: u8;
        let mut end_high: u8;
        loop {
            unsafe {
                pit_mode_port.write(PIT_MODE_CHANNEL_0 | PIT_MODE_ACCESS_LATCH);
                end_low = pit_channel_0_port.read();
                end_high = pit_channel_0_port.read();
            }

            if end_high <= 0x80 {
                break;
            }
        }

        // Get the end TSC value
        let mut tsc_end_time;
        unsafe {
            tsc_end_time = _rdtsc();
        }

        // Calculate the difference between the value
        let cycles = (tsc_end_time - tsc_start_time) as u64;
        let end_record = ((end_high as u16) << 8) | (end_low as u16);
        let start_record = ((start_high as u16) << 8) | (start_low as u16);
        let ticks = end_record.wrapping_sub(start_record);

        // Calculate frequency
        self.tsc_cycles_per_msec = (cycles * PIT_FREQUENCY) / (ticks as u64 * 1000);
    }
}
