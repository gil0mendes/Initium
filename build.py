#!/usr/bin/env python3
# from utils.build.toolchain import ToolchainManager
'Script used automate some common tasks when developing for Initium.'

import os
import argparse
import sys
import json
import shutil
import subprocess as sp
import urllib.request
from pathlib import Path

# Add the path to our build utilities to the path
sys.path = [os.path.abspath(os.path.join('utils', 'build'))] + sys.path


# Target variables
ARCH = 'x86_64'
PLATFORM = 'efi'
TARGET = ARCH + '-unknown-' + PLATFORM

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

SETTINGS = {
    # Architecture to build for
    'arch': 'x86_64',
    # Print commands before tunning them
    'verbose': False,
    # Run QEMU without, showing GUI
    'headless': False,
    # Configuration to build
    'config': CONFIG,
    # QEMU executable to use
    # Indexed by the `arch` setting
    'qemu_binary': {
        'x86_64': 'qemu-system-x86_64',
        'aarch64': 'qemu-system-aarch64',
    },
    # Path to directory containing `OVMF_{CODE/VARS}.fd` (for x86_64),
    # or `*-pflash.raw` (for AArch64).
    # `find_ovmf` function will try to find one if this isn't specified.
    'ovmf_dir': None,
}

# Path to target directory. If None, it will be initialized with information
# from cargo metadata at the first time target_dir function is invoked.
TARGET_DIR = None


def clean():
    'Clean generated objects'
    sp.run(['xargo', 'clean'])
    shutil.rmtree(BUILD_DIR)


def target_dir():
    """
    Returns the target directory
    """
    global TARGET_DIR
    if TARGET_DIR is None:
        cmd = ['cargo', 'metadata', '--format-version=1']
        result = sp.run(cmd, stdout=sp.PIPE, check=True)
        TARGET_DIR = Path(json.loads(result.stdout)['target_directory'])
    return TARGET_DIR


def get_target_triple():
    arch = SETTINGS['arch']
    return f'{arch}-unknown-efi'


def build_dir():
    """
    Returns the directory where Cargo places the build artifacts
    """
    return target_dir() / get_target_triple() / SETTINGS['config']


def esp_dir():
    'Returns the directory where we will build the emulated UEFI system partition'
    return build_dir() / 'esp'


def iso_file():
    'Return the path where we will put the ISO file'
    return build_dir() / ISO_FILE_NAME


def run_tool(tool, *flags):
    'Runs Cargo with certain arguments'

    target = get_target_triple()
    # Custom targets need to be given by relative path, instead of only by name
    # We need to append a `.json` to turn the triple into a path
    if SETTINGS['arch'] == 'aarch64':
        target += '.json'

    cmd = ['cargo', tool, '--target', target, *flags]

    if SETTINGS['verbose']:
        print(' '.join(cmd))

    sp.run(cmd, check=True)


def run_build(*flags):
    """
    Runs cargo-build with certain arguments.
    """
    run_tool('build', *flags)


def build_command(*test_flags):
    """
    Builds Initium bootloader
    """

    build_args = [
        '--package', 'initium',
        *test_flags,
    ]

    if SETTINGS['config'] == 'release':
        build_args.append('--release')

    run_build(*build_args)

    # Copy the build test runner file to the right directory for runnings tests
    build_file = build_dir() / 'initium.efi'

    # Create build folder
    boot_dir = esp_dir() / 'EFI' / 'Boot'
    boot_dir.mkdir(parents=True, exist_ok=True)

    arch = SETTINGS['arch']
    if arch == 'x86_64':
        output_file = boot_dir / 'BootX64.efi'

    cmd = ('rust-lld -flavor gnu -T./platform/efi/linker.ld -o ./target/x86_64-unknown-efi/debug/esp/EFI/Boot/BootX64.efi ./target/x86_64-unknown-efi/debug/libinitium.a').split(' ')
    sp.run(cmd, stdout=sp.PIPE, check=True)

    # Copy the built EFI application to the right directory for running tests.
    # Build the final EFI binary, by translating into a EFI format
    built_file = CARGO_BUILD_DIR / 'initium.efi'

    cmd = ('objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.\* -j .rela.\* -j .rel\* -j .rela\* -j .reloc -Opei-x86-64 --subsystem=efi-app ' +
           './target/x86_64-unknown-efi/debug/esp/EFI/Boot/BootX64.efi').split(' ')
    sp.run(cmd, stdout=sp.PIPE, check=True)

    # shutil.copy2(built_file, output_file)

    # Write startup file to load into loader automatically
    startup_file = open(esp_dir() / "startup.nsh", "w")
    startup_file.write("\EFI\BOOT\BOOTX64.EFI")
    startup_file.close()


def build_iso():
    sp.run(['mkisofs', '-V', 'Initium', '-o', iso_file(), BUILD_DIR])


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
        # '-machine', 'q35,accel=kvm:tcg', - only for Linux
        '-machine', 'type=q35,accel=hvf',

        # Allocate some memory.
        '-m', '128M',

        # Set up OVMF.
        '-drive', 'if=pflash,format=raw,readonly,file=OVMF_CODE.fd',
        '-drive', 'if=pflash,format=raw,file=OVMF_VARS-1024x768.fd',

        # Mount a local directory as a FAT partition
        '-drive', f'format=raw,file=fat:rw:{esp_dir()}',

        # Enable serial
        #
        # Connect the serial port to the host. OVMF is kind enough to connect
        # the UEFI stdout and stdin to that port too.
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
    # toolchain = ToolchainManager(config)
    # toolchain.update()


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
    subparsers.add_parser("check")

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
