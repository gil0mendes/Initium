# Initium

> 🚨 This branch contains a work in process rewriting of the current features to Rust. If you want to see the older version you can find it [here](https://github.com/gil0mendes/Initium).

## Table of Contents

```text
  1 Introduction
  2 Using Initium
    2.1 Requirements
    2.2 Build
    2.3 Test
```

---

## 1 Introduction

Intinitum is a bootloader for the x86 processors (the goals is to add ARM and RISC-V). Currently, supports Unified Extended Firmware Interface(UEFI). This bootloader was originally created for the [Infinity OS](https://github.com/gil0mendes/Infinity-OS) project.

## 2 Build

### 2.1 Requirements

Here is a list of required tools to build Initium. Note that, currently, the build system only support Unix-like Operating Systems.

-   [Rust](https://www.rust-lang.org/)
-   [QEMU](https://www.qemu.org/)
-   [Python 3](https://www.python.org/)

### 2.2 Build

With the dependencies correctly installed the build process is very simples. On the root of the project there is a `build.py` file with some useful commands to assist with the build process.

To compile the bootloader you must simply run the following command:

```shell
$ ./build.py build
```

The first build will take longer to download and compile the dependencies, but the consequent ones will be incremental.

### 2.3 Test

There is a simples test environment that uses QEMU to test the Initium. The build script also has a command to launch it:

```shell
$ ./build.py run
```

## License

[MIT](https://opensource.org/licenses/MIT)

Copyright (c) 2014-present, [Gil Mendes](https://gil0mendes.io)
