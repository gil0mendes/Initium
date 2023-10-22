# Initium

> 🚨 This branch contains a work in process rewriting of the current features to Rust. If you want to see the older version you can find it [here](https://github.com/gil0mendes/Initium).

- [Initium](#initium)
  - [Introduction](#introduction)
  - [Development](#development)
  - [Build](#build)
    - [Requirements](#requirements)
    - [Build](#build-1)
    - [Test](#test)
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

## Build

### Requirements

Here is a list of required tools to build Initium. Note that, currently, the build system only support Unix-like Operating Systems.

- [Rust](https://www.rust-lang.org/)
- [QEMU](https://www.qemu.org/)
- [Python 3](https://www.python.org/)

Before moving with the build, please fetch all submodules:

```shell
git submodules update --init
```

### Build

With the dependencies correctly installed the build process is very simples. On the root of the project there is a `build.py` file with some useful commands to assist with the build process.

To compile the bootloader you must simply run the following command:

```shell
$ ./build.py build
```

The first build will take longer to download and compile the dependencies, but the consequent ones will be incremental.

### Test

There is a simples test environment that uses QEMU to test the Initium. The build script also has a command to launch it:

```shell
$ ./build.py run
```

## License

[MIT](https://opensource.org/licenses/MIT)

Copyright (c) 2014-present, [Gil Mendes](https://gil0mendes.io)
