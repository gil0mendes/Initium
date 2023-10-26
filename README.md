# Initium

> 🚨 This branch contains a work in process rewriting of the current features to Rust. If you want to see the older version you can find it [here](https://github.com/gil0mendes/Initium).

- [Initium](#initium)
  - [Introduction](#introduction)
  - [Development](#development)
  - [Build and Testing](#build-and-testing)
  - [License](#license)

---

## Introduction

Intinitum is a bootloader for the x86 processors (the goals is to add ARM and RISC-V). Currently, supports Unified Extended Firmware Interface(UEFI). This bootloader was originally created for the [Infinity OS](https://github.com/gil0mendes/Infinity-OS) project.

## Development

For the development is recommended to use [nix](https://nixos.org/) to load all the necessary tools for the development and testing.

We have a `shell.nix` file that declares a nix shell that loads the required tools, to make use of them run the following command:

```sh
nix-shell
```

> **📔 Note:** If you use VSCode for the development you can install the Nix IDE extension that will automatically load the environment for you.

## Build and Testing

Use the `cargo xtask` command to build and test Initium.

Available commands:

- `build`: build all the Initium packages and binary
  - `--release`: build in release mode
- `run`: run Initium in QEMU
  - `--disable-kvm`: disable hardware accelerated virtualization support in QEMU.
  - `--release`: build in release mode

## License

[MIT](https://opensource.org/licenses/MIT)

Copyright (c) 2014-present, [Gil Mendes](https://gil0mendes.io)
