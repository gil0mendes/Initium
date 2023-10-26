use std::{
    env,
    ffi::OsString,
    io::{BufRead, BufReader, Cursor, Read, Write},
    path::{Path, PathBuf},
    process::{Child, Command, Stdio},
};

use anyhow::{bail, Context, Result};
use regex::bytes::Regex;
use serde_json::Value;
use sha2::{Digest, Sha256};
use tar::Archive;
use tempfile::TempDir;
use ureq::Agent;

use crate::{
    arch::InitiumArch, disk::create_mbr_test_disk, opt::QemuOpt, pipe::Pipe, platform,
    util::command_to_string,
};
#[cfg(target_os = "linux")]
use {std::fs::Permissions, std::os::unix::fs::PermissionsExt};

/// Name of the ovmf-prebuilt release tag.
const OVMF_PREBUILT_TAG: &str = "edk2-stable202211-r1";

/// SHA-256 hash of the release tarball.
const OVMF_PREBUILT_HASH: &str = "b085cfe18fd674bf70a31af1dc3e991bcd25cb882981c6d3523d81260f1e0d12";

/// Directory into which the prebuilts will be download (relative to the repo root).
const OVMF_PREBUILT_DIR: &str = "target/ovmf";

/// Environment variable for overriding the path of the OVMF code file.
const ENV_VAR_OVMF_CODE: &str = "OVMF_CODE";

/// Environment variable for overriding the path of the OVMF vars file.
const ENV_VAR_OVMF_VARS: &str = "OVMF_VARS";

/// Environment variable for overriding the path of the OVMF shell file.
const ENV_VAR_OVMF_SHELL: &str = "OVMF_SHELL";

/// Download `url` and return the raw data.
fn download_url(url: &str) -> Result<Vec<u8>> {
    let agent: Agent = ureq::AgentBuilder::new()
        .user_agent("uefi-rs-ovmf-downloader")
        .build();

    // Limit the size of the download.
    let max_size_in_bytes = 4 * 1024 * 1024;

    // Download the file.
    println!("downloading {url}");
    let resp = agent.get(url).call()?;
    let mut data = Vec::with_capacity(max_size_in_bytes);
    resp.into_reader()
        .take(max_size_in_bytes.try_into().unwrap())
        .read_to_end(&mut data)?;
    println!("received {} bytes", data.len());

    Ok(data)
}

// Extract the tarball's files into `prebuilt_dir`.
//
// `tarball_data` is raw decompressed tar data.
fn extract_prebuilt(tarball_data: &[u8], prebuilt_dir: &Path) -> Result<()> {
    let cursor = Cursor::new(tarball_data);
    let mut archive = Archive::new(cursor);

    // Extract each file entry.
    for entry in archive.entries()? {
        let mut entry = entry?;

        // Skip directories.
        if entry.size() == 0 {
            continue;
        }

        let path = entry.path()?;
        // Strip the leading directory, which is the release name.
        let path: PathBuf = path.components().skip(1).collect();

        let dir = path.parent().unwrap();
        let dst_dir = prebuilt_dir.join(dir);
        let dst_path = prebuilt_dir.join(path);
        println!("unpacking to {}", dst_path.display());
        fs_err::create_dir_all(dst_dir)?;
        entry.unpack(dst_path)?;
    }

    Ok(())
}

/// Update the local copy of the prebuilt OVMF files. Does nothing if the local
/// copy is already up to date.
fn update_prebuilt() -> Result<PathBuf> {
    let prebuilt_dir = Path::new(OVMF_PREBUILT_DIR);
    let hash_path = prebuilt_dir.join("sha256");

    // Check if the hash file already has the expected hash in it. If so, assume
    // that we've already got the correct prebuilt downloaded and unpacked.
    if let Ok(current_hash) = fs_err::read_to_string(&hash_path) {
        if current_hash == OVMF_PREBUILT_HASH {
            return Ok(prebuilt_dir.to_path_buf());
        }
    }

    let base_url = "https://github.com/rust-osdev/ovmf-prebuilt/releases/download";
    let url = format!(
        "{base_url}/{release}/{release}-bin.tar.xz",
        release = OVMF_PREBUILT_TAG
    );

    let data = download_url(&url)?;

    // Validate the hash.
    let actual_hash = format!("{:x}", Sha256::digest(&data));
    if actual_hash != OVMF_PREBUILT_HASH {
        bail!(
            "file hash {actual_hash} does not match {}",
            OVMF_PREBUILT_HASH
        );
    }

    // Unpack the tarball.
    println!("decompressing tarball");
    let mut decompressed = Vec::new();
    let mut compressed = Cursor::new(data);
    lzma_rs::xz_decompress(&mut compressed, &mut decompressed)?;

    // Clear out the existing prebuilt dir, if present.
    let _ = fs_err::remove_dir_all(prebuilt_dir);

    // Extract the files.
    extract_prebuilt(&decompressed, prebuilt_dir)?;

    // Rename the x64 directory to x86_64, to match `Arch::as_str`.
    fs_err::rename(prebuilt_dir.join("x64"), prebuilt_dir.join("x86_64"))?;

    // Write out the hash file. When we upgrade to a new release of
    // ovmf-prebuilt, the hash will no longer match, triggering a fresh
    // download.
    fs_err::write(&hash_path, actual_hash)?;

    Ok(prebuilt_dir.to_path_buf())
}

#[derive(Clone, Copy, Debug)]
enum OvmfFileType {
    Code,
    Vars,
    Shell,
}

impl OvmfFileType {
    fn as_str(&self) -> &'static str {
        match self {
            Self::Code => "code",
            Self::Vars => "vars",
            Self::Shell => "shell",
        }
    }

    fn extension(&self) -> &'static str {
        match self {
            Self::Code | Self::Vars => "fd",
            Self::Shell => "efi",
        }
    }

    /// Get a user-provided path for the given OVMF file type.
    ///
    /// This uses the command-line arg if present, otherwise it falls back to an
    /// environment variable. If neither is present, returns `None`.
    fn get_user_provided_path(self, opt: &QemuOpt) -> Option<PathBuf> {
        let opt_path;
        let var_name;
        match self {
            Self::Code => {
                opt_path = &opt.ovmf_code;
                var_name = ENV_VAR_OVMF_CODE;
            }
            Self::Vars => {
                opt_path = &opt.ovmf_vars;
                var_name = ENV_VAR_OVMF_VARS;
            }
            Self::Shell => {
                opt_path = &None;
                var_name = ENV_VAR_OVMF_SHELL;
            }
        }
        if let Some(path) = opt_path {
            Some(path.clone())
        } else {
            env::var_os(var_name).map(PathBuf::from)
        }
    }
}

struct OvmfPaths {
    code: PathBuf,
    vars: PathBuf,
    shell: PathBuf,
}

impl OvmfPaths {
    /// Search for an OVMF file (either code or vars).
    ///
    /// There are multiple locations where a file is searched at in the following
    /// priority:
    /// 1. Command-line arg
    /// 2. Environment variable
    /// 3. Prebuilt file (automatically downloaded)
    fn find_ovmf_file(
        file_type: OvmfFileType,
        opt: &QemuOpt,
        arch: InitiumArch,
    ) -> Result<PathBuf> {
        if let Some(path) = file_type.get_user_provided_path(opt) {
            // The user provided an exact path to use; verify that it
            // exists.
            if path.exists() {
                Ok(path)
            } else {
                bail!(
                    "ovmf {} file does not exist: {}",
                    file_type.as_str(),
                    path.display()
                );
            }
        } else {
            let prebuilt_dir = update_prebuilt()?;

            Ok(prebuilt_dir.join(format!(
                "{arch}/{}.{}",
                file_type.as_str(),
                file_type.extension()
            )))
        }
    }

    /// Find path to OVMF files by the strategy documented for
    /// [`Self::find_ovmf_file`].
    fn find(opt: &QemuOpt, arch: InitiumArch) -> Result<Self> {
        let code = Self::find_ovmf_file(OvmfFileType::Code, opt, arch)?;
        let vars = Self::find_ovmf_file(OvmfFileType::Vars, opt, arch)?;
        let shell = Self::find_ovmf_file(OvmfFileType::Shell, opt, arch)?;

        Ok(Self { code, vars, shell })
    }
}

enum PflashMode {
    ReadOnly,
    ReadWrite,
}

fn add_pflash_args(cmd: &mut Command, file: &Path, mode: PflashMode) {
    // Build the argument as an OsString to avoid requiring a UTF-8 path.
    let mut arg = OsString::from("if=pflash,format=raw,readonly=");
    arg.push(match mode {
        PflashMode::ReadOnly => "on",
        PflashMode::ReadWrite => "off",
    });
    arg.push(",file=");
    arg.push(file);

    cmd.arg("-drive");
    cmd.arg(arg);
}

pub struct Io {
    reader: BufReader<Box<dyn Read + Send>>,
    writer: Box<dyn Write + Send>,
}

impl Io {
    pub fn new<R, W>(r: R, w: W) -> Self
    where
        R: Read + Send + 'static,
        W: Write + Send + 'static,
    {
        Self {
            reader: BufReader::new(Box::new(r)),
            writer: Box::new(w),
        }
    }

    fn read_line(&mut self) -> Result<String> {
        let mut line = String::new();
        let num = self.reader.read_line(&mut line)?;
        if num == 0 {
            bail!("EOF reached");
        }
        Ok(line)
    }

    fn read_json(&mut self) -> Result<Value> {
        let line = self.read_line()?;
        Ok(serde_json::from_str(&line)?)
    }

    fn write_all(&mut self, s: &str) -> Result<()> {
        self.writer.write_all(s.as_bytes())?;
        self.writer.flush()?;
        Ok(())
    }

    fn write_json(&mut self, json: Value) -> Result<()> {
        // Note: it's important not to add anything after the JSON data
        // such as a trailing newline. On Windows, QEMU's pipe reader
        // will hang if that happens.
        self.write_all(&json.to_string())
    }
}

fn process_qemu_io(mut serial_io: Io, tmp_dir: &Path) -> Result<()> {
    // This regex is used to detect and strip ANSI escape codes. These
    // escapes are added by the console output protocol when writing to
    // the serial device.
    let ansi_escape = Regex::new(r"(\x9b|\x1b\[)[0-?]*[ -/]*[@-~]").expect("invalid regex");

    while let Ok(line) = serial_io.read_line() {
        // Strip whitespace and ANSI escape codes
        let line = line.trim_end();
        let line = ansi_escape.replace_all(line.as_bytes(), &b""[..]);
        let line = String::from_utf8(line.into()).expect("line is not utf8");

        println!("{line}");
    }

    Ok(())
}

/// Create an EFI boot directory to pass into QEMU
fn build_esp_dir(opt: &QemuOpt, ovmf_paths: &OvmfPaths) -> Result<PathBuf> {
    let build_mode = if opt.build_mode.release {
        "release"
    } else {
        "debug"
    };
    let build_dir = Path::new("target")
        .join(opt.target.as_triple())
        .join(build_mode);
    let esp_dir = build_dir.join("esp");

    // Create boot dir.
    let boot_dir = esp_dir.join("EFI").join("Boot");
    if !boot_dir.exists() {
        fs_err::create_dir_all(&boot_dir)?;
    }

    let boot_file_name = match *opt.target {
        InitiumArch::X86_64 => "BootX64.efi",
    };

    // For the test-runner, launch the `shell_launcher` binary first. That will then launch the UEFI shell, and run
    // the `uefi-test-runner` inside the shell. This allows the test-runner to test protocols that use the shell.
    let initium_boot = build_dir.join("initium.efi");
    fs_err::copy(initium_boot, boot_dir.join(boot_file_name))?;

    // fs_err::copy(&ovmf_paths.shell, boot_dir.join("shell.efi"))?;

    // let test_runner = build_dir.join("uefi-test-runner.efi");
    // fs_err::copy(test_runner, boot_dir.join("test_runner.efi"))?;

    Ok(esp_dir)
}

/// Wrap a child process to automatically kill it when dropped.
struct ChildWrapper(Child);

impl Drop for ChildWrapper {
    fn drop(&mut self) {
        // Do nothing if child has already exited (this call doesn't block).
        if matches!(self.0.try_wait(), Ok(Some(_))) {
            return;
        }

        // Try to stop the process, then wait for it to exit. Log errors
        // but otherwise ignore.
        if let Err(err) = self.0.kill() {
            eprintln!("failed to kill process: {err}");
        }
        if let Err(err) = self.0.wait() {
            eprintln!("failed to wait for process exit: {err}");
        }
    }
}

pub fn run_qemu(arch: InitiumArch, opt: &QemuOpt) -> Result<()> {
    let qemu_exe = match arch {
        InitiumArch::X86_64 => "qemu-system-x86_64",
    };
    let mut cmd = Command::new(qemu_exe);

    // Disable default devices.
    // QEMU by defaults enable a ton of devices which slow down boot
    cmd.arg("-nodefaults");

    // add paravirtualized device to serve as random number generator
    cmd.args(["-device", "virtio-rng-pci"]);

    // Set the boot menu timeout to zero.
    cmd.args(["-boot", "menu=on,splash-time=0"]);

    match arch {
        InitiumArch::X86_64 => {
            // Use a modern machine.
            cmd.args(["-machine", "q35"]);

            // Allocate some memory.
            cmd.args(["-m", "256M"]);

            // Graphics device.
            cmd.args(["-vga", "std"]);

            // Enable hardware-accelerated virtualization if possible.
            if platform::is_linux() && !opt.disable_kvm {
                cmd.arg("--enable-kvm");
            }
        }
    };

    let tmp_dir = TempDir::new()?;
    let tmp_dir = tmp_dir.path();

    // Set up OVMF.
    let ovmf_paths = OvmfPaths::find(opt, arch)?;

    // Make a copy of the OVMF vars file so that it can be used read+write without modifying the original.
    let ovmf_vars = tmp_dir.join("ovmf_vars");
    fs_err::copy(&ovmf_paths.vars, &ovmf_vars)?;

    // Necessary, as for example on NixOS, the files are read-only inside the Nix store.
    #[cfg(target_os = "linux")]
    fs_err::set_permissions(&ovmf_vars, Permissions::from_mode(0o666))?;

    add_pflash_args(&mut cmd, &ovmf_paths.code, PflashMode::ReadOnly);
    add_pflash_args(&mut cmd, &ovmf_vars, PflashMode::ReadWrite);

    // Mount a local directory as a FAT partition.
    cmd.arg("-drive");
    let mut drive_arg = OsString::from("format=raw,file=fat:rw:");
    let esp_dir = build_esp_dir(opt, &ovmf_paths)?;
    drive_arg.push(esp_dir);
    cmd.arg(drive_arg);

    let test_disk = tmp_dir.join("test_disk.fat.img");
    create_mbr_test_disk(&test_disk)?;

    cmd.arg("-drive");
    let mut drive_arg = OsString::from("format=raw,file=");
    drive_arg.push(test_disk.clone());
    cmd.arg(drive_arg);

    // Configure serial that will print logs to stdout
    let serial_pipe = Pipe::new(tmp_dir, "serial")?;
    cmd.args(["-serial", serial_pipe.qemu_arg()]);

    // Print the actual used QEMU command for running the test.
    println!("{}", command_to_string(&cmd));

    cmd.stdin(Stdio::piped());
    cmd.stdout(Stdio::piped());
    let mut child = ChildWrapper(cmd.spawn().context("failed to launch qemu")?);

    let serial_io = serial_pipe.open_io()?;

    // Capture the result to check it, but first wait for the child to exit
    let res = process_qemu_io(serial_io, tmp_dir);
    let status = child.0.wait()?;

    // Propagate earlier error if necessary
    res?;

    // Get qemu's exit code if possible, or return an error if terminated by a signal.
    let qemu_exit_code = status
        .code()
        .context(format!("qemu was terminated by a signal: {status:?}"))?;

    println!("Controller> Exist code: {}", qemu_exit_code);

    Ok(())
}
