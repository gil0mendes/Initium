#!/usr/bin/env python3
'Script used automate some common tasks when developing for Initium.'

import sys
import os
import shutil
import subprocess as sp
import urllib.request
from pathlib import Path

# Add the path to our build utilities to the path
sys.path = [os.path.abspath(os.path.join('utils', 'build'))] + sys.path

from toolchain import ToolchainManager

## Configurable settings
BINUTILS_VER = '2.28.1'

# Target variables
ARCH = 'x86_64'
PLATFORM = 'efi'
TARGET = ARCH + '-initium-' + PLATFORM

# Configuration to build
CONFIG = 'debug'

# Tools
QEMU = 'qemu-system-' + ARCH

# Path to workspace directory
WORKSPACE_DIR = Path(__file__).resolve().parents[0]

# Path to directory containing OVMF files
OVMF_FW = WORKSPACE_DIR / '.ovmf-amd64.bin'

XARGO_BUILD_DIR = WORKSPACE_DIR / 'target' / TARGET / CONFIG
XARGO_TARGETS_DIR = WORKSPACE_DIR / 'targets'
BUILD_DIR = WORKSPACE_DIR / 'build'
TOOLS_DIR = BUILD_DIR / 'tools'
ISO_DIR = BUILD_DIR / 'iso'
ISO_FILE_NAME = TARGET + '.iso'
ISO_FILE = BUILD_DIR / ISO_FILE_NAME

def clean():
    'Clean generated objects'
    sp.run(['xargo', 'clean'])
    shutil.rmtree(BUILD_DIR)

def run_xargo(verb, *flags):
    'Runs Xargo with certain arguments'
    sp.run(
        ['xargo', verb, '--target', TARGET, *flags],
        env = dict(os.environ, RUST_TARGET_PATH = XARGO_TARGETS_DIR)
    ).check_returncode()

def build():
    'Builds Initium bootloader'

    # Build Initium
    run_xargo('build', '--package', 'initium')
    built_file = XARGO_BUILD_DIR / 'libinitium.a'
    object_file = BUILD_DIR / 'initium.o'

    # Convert library into an object file to be linked
    sp.run([
        'ar',
        ''
    ])

    # Link bootloader and produce PE compatible executable
    sp.run([
        'ar',
        '--oformat', 'pei-x86-64',
		'--dll',
		'--image-base', '0',
		'--section-alignment', '32',
		'--file-alignment', '32',
		'--major-os-version', '0',
		'--minor-os-version', '0',
		'--major-image-version', '0',
		'--minor-image-version', '0',
		'--major-subsystem-version', '0',
		'--minor-subsystem-version', '0',
		'--subsystem', '10',
		'--heap', '0,0',
		'--stack', '0,0',
		'--pic-executable',
		'--entry', 'uefi_main',
		'--no-insert-timestamp',
        built_file, '-o', object_file
    ]).check_returncode()

    # linked_file = BUILD_DIR / 'initium.bin'
    # sp.run([
    #     'ld.lld',
    #     *LDFLAGS,
    #     '-T', LINKER_SCRIPT,
    #     '-L', XARGO_BUILD_DIR,
    #     '-l', 'initium',
    #     '-o', linked_file,
    #     built_file
    # ])

    # Create build folder
    boot_dir = BUILD_DIR / 'EFI' / 'BOOT'
    boot_dir.mkdir(parents = True, exist_ok=True)

    # Copy output into build dir    
    output_file = boot_dir / 'BootX64.efi'
    shutil.copy2(built_file, output_file)

def build_iso():
    sp.run(['mkisofs', '-V', 'Initium', '-o', ISO_FILE, BUILD_DIR])

def run_qemu():
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

# def toolchain():
#     'Compile toolchain'
#     # Remove previous tools
#     if os.path.exists(TOOLS_DIR):
#         shutil.rmtree(TOOLS_DIR)

#     # Create the new folder here the tools will be placed
#     os.makedirs(TOOLS_DIR / 'downloads')

#     # Download Binutils
#     url = f'https://ftp.gnu.org/gnu/binutils/binutils-{BINUTILS_VER}.tar.xz'
#     urllib.request.urlretrieve(url, TOOLS_DIR / 'downloads' / 'binutils.tar.xz')

#     # Extract

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

def main(args) -> int:
    """ Runs the user-requested actions. """

    # We needs at least one parameter
    if len(args) < 2:
        print("Expected at least one parameter (the commands to run):\n - toolchain\n - build\n - run")
        return 1

    # Get all given commands
    commands = args[1:]

    # List of available commands
    KNOWN_COMMANDS = {
        'toolchain': toolchain_command
    }

    for cmd in commands:
        if cmd in KNOWN_COMMANDS:
            try:
                KNOWN_COMMANDS[cmd]()
            except sp.CalledProcessError:
                return 1
        else:
            print("Unknown verb:", cmd)
            return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
