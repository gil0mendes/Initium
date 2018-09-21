#!/usr/bin/env python3
'Script used automate some common tasks when developing for Initium.'

import os
import argparse
import sys
import shutil
import subprocess as sp
import urllib.request
from pathlib import Path

# Add the path to our build utilities to the path
sys.path = [os.path.abspath(os.path.join('utils', 'build'))] + sys.path

from toolchain import ToolchainManager
from utils import execute, makedirs, remove

# Target variables
ARCH = 'x86_64'
PLATFORM = 'efi'
TARGET = ARCH + '-initium-' + PLATFORM

# Configuration to build
CONFIG = 'debug'

# Tools
QEMU = 'qemu-system-' + ARCH

# Set to `True` or use the `--verbose` argument to print commands.
VERBOSE = False

# Path to workspace directory
WORKSPACE_DIR = Path(__file__).resolve().parents[0]
BUILD_DIR = WORKSPACE_DIR / 'build'
TOOLS_DIR = BUILD_DIR / 'tools'
TOOLS_PREFIX = TOOLS_DIR / 'generic' / 'x86_64-efi-pe' / 'bin'

# Tools
AR = TOOLS_PREFIX / 'ar'
LD = TOOLS_PREFIX / 'ld'

# Path to directory containing OVMF files
OVMF_FW = WORKSPACE_DIR / '.ovmf-amd64.bin'

CARGO_BUILD_DIR = WORKSPACE_DIR / 'target' / TARGET / CONFIG
CARGO_TARGETS_DIR = WORKSPACE_DIR / 'targets'
ISO_DIR = BUILD_DIR / 'iso'
ISO_FILE_NAME = TARGET + '.iso'
ISO_FILE = BUILD_DIR / ISO_FILE_NAME

def clean():
    'Clean generated objects'
    sp.run(['xargo', 'clean'])
    shutil.rmtree(BUILD_DIR)

def run_xbuild(*flags):
    'Runs Cargo XBuild with certain arguments'

    cmd = ['cargo', 'xbuild', '--target', TARGET, *flags]

    if VERBOSE:
        print(' '.join(cmd))

    sp.run(cmd).check_returncode()

def build_command():
    """
    Builds Initium bootloader
    """

    run_xbuild('--package', 'initium')

    # Create build folder
    boot_dir = BUILD_DIR / 'EFI' / 'BOOT'
    boot_dir.mkdir(parents=True, exist_ok=True)

    # Copy the built EFI application to the right directory
    # for running tests.
    built_file = CARGO_BUILD_DIR / 'initium.efi'

    output_file = boot_dir / 'BootX64.efi'
    shutil.copy2(built_file, output_file)

def build_iso():
    sp.run(['mkisofs', '-V', 'Initium', '-o', ISO_FILE, BUILD_DIR])

def run_command():
    'Runs the code in QEMU'

    qemu_flags = [
        '-s',

        # Disable default devices
        # QEMU by default enables a ton of devices which slow down boot.
        '-nodefaults',

        # Use a standard VGA for graphics
        '-vga', 'std',

        # Use a modern machine, with acceleration if possible.
        '-machine', 'q35,accel=kvm:tcg',

        # Allocate some memory.
        '-m', '128M',

        # Set up OVMF.
        '-drive', 'if=pflash,format=raw,readonly,file=OVMF_CODE.fd',
	    '-drive', 'if=pflash,format=raw,file=OVMF_VARS-1024x768.fd',

        # Mount a local directory as a FAT partition
        '-drive', f'format=raw,file=fat:rw:{BUILD_DIR}',

        # Enable serial
        '-serial', 'stdio',

        # Setup monitor
        '-monitor', 'vc:1024x768',
        
        # Create AHCI controller
        '-device', 'ahci,id=ahci,multifunction=on'
    ]

    sp.run([QEMU] + qemu_flags).check_returncode()

def toolchain_command():
    # TODO: implement a proper config mechanism
    config = {
        'ARCH': ARCH,
        'PLATFORM': PLATFORM,
        'TOOLCHAIN_DIR': TOOLS_DIR,
        'TOOLCHAIN_TARGET': 'x86_64-efi-pe',
        'TOOLCHAIN_MAKE_JOBS': 16
    }

    # Initialise the toolchain manager and add the toolchain build target.
    toolchain = ToolchainManager(config)
    toolchain.update()

def main(args):
    "Runs the user-requested actions."

    # Clear any Rust flags which might affect the build.
    os.environ["RUSTFLAGS"] = ""
    os.environ["RUST_TARGET_PATH"] = str(CARGO_TARGETS_DIR)

    usage = "%(prog)s verb [options]"
    desc = "Build script for Initium"

    parser = argparse.ArgumentParser(usage=usage, description=desc)

    parser.add_argument("--verbose", "-v", action="store_true")

    subparsers = parser.add_subparsers(dest="verb")
    toolchain_parser = subparsers.add_parser("toolchain")
    build_parser = subparsers.add_parser("build")
    run_parser = subparsers.add_parser("run")

    opts = parser.parse_args()

    # Check if we need to enable verbose mode
    global VERBOSE
    VERBOSE = VERBOSE or opts.verbose

    if opts.verb == "toolchain":
        toolchain_command()
    elif opts.verb == "build":
        build_command()
    elif opts.verb == "run":
        run_command()
    else:
        raise ValueError(f"Unknown verb {opts.verb}")

if __name__ == '__main__':
    sys.exit(main(sys.argv))
