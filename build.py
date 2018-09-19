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

XARGO_BUILD_DIR = WORKSPACE_DIR / 'target' / TARGET / CONFIG
XARGO_TARGETS_DIR = WORKSPACE_DIR / 'targets'
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
    print(cmd)

    sp.run(cmd, env = dict(os.environ, RUST_TARGET_PATH = XARGO_TARGETS_DIR)).check_returncode()

def build_command():
    """
    Builds Initium bootloader
    """

    # Build Initium
    built_file = XARGO_BUILD_DIR / 'libinitium.a'
    run_xbuild('--package', 'initium')
    
    # Extract object from library
    # extract_folder = BUILD_DIR / 'temp_extract'
    # object_file = BUILD_DIR / 'initium.o'

    # makedirs(extract_folder)
    # execute(f'{AR} -x {built_file}', extract_folder)

    # Link all objects into one file
    # execute(f'ld.lld -r *.o -o {object_file}', extract_folder)
    # remove(extract_folder)

    # Link bootloader and produce PE compatible executable
    # efi_executable = BUILD_DIR / 'initium.efi'
    # command = (
    #     f'{LD} '
    #     '--oformat pei-x86-64 '
	# 	'--dll '
	# 	'--image-base 0 '
	# 	'--section-alignment 32 '
	# 	'--file-alignment 32 '
	# 	'--major-os-version 0 '
	# 	'--minor-os-version 0 '
	# 	'--major-image-version 0 '
	# 	'--minor-image-version 0 '
	# 	'--major-subsystem-version 0 '
	# 	'--minor-subsystem-version 0 '
	# 	'--subsystem 10 '
	# 	'--heap 0,0 '
	# 	'--stack 0,0 '
	# 	'--pic-executable '
	# 	'--entry uefi_start '
	# 	'--no-insert-timestamp '
    #     f'{object_file} '
    #     f'-o {efi_executable}'
    # )
    # execute(command)

    # # Create build folder
    # boot_dir = BUILD_DIR / 'EFI' / 'BOOT'
    # makedirs(boot_dir)

    # # Copy output into build dir    
    # output_file = boot_dir / 'BootX64.efi'
    # shutil.copy2(efi_executable, output_file)

def build_iso():
    sp.run(['mkisofs', '-V', 'Initium', '-o', ISO_FILE, BUILD_DIR])

def run_command():
    'Runs the code in QEMU'

    qemu_flags = [
        '-s',
        # Disable default devices
        '-nodefaults',
        # Enable serial
        '-serial', 'stdio',
        # Use a standard VGA for graphics
        '-vga', 'std',
        # Setup monitor
        '-monitor', 'vc:1024x768',
        # Allocate some memory.
        '-m', '512M',
        # Set up OVMF.
        '-drive', f'if=pflash,format=raw,unit=0,file={OVMF_FW},readonly=on',
        # Create AHCI controller
        '-device', 'ahci,id=ahci,multifunction=on',
        # Mount a local directory as a FAT partition
        '-drive', f'file=fat:rw:{BUILD_DIR}'
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
