# Initium

### Table of Contents

	1 Introduction
	2 Using Initium
		2.1 Configure the Kernel
		2.2 Build Initium
			2.2.1 Options
      2.2.2 Test

## 1 Introduction

Initium is a bootloader for the x86 and ARM processors. This bootloader supports the BIOS legacy PC boot system and the Unified Extended Firmware Interface (UEFI). This bootloader was originally created for the [Infinity OS](https://github.com/gil0mendes/Infinity-OS) project.

Click in the next link to see the [Initium Boot Protocol](https://github.com/gil0mendes/Initium/blob/master/documentation/initium-protocol.md).

Thanks for using Initium!

## 2 Using Initium

### 2.1 Configure

To use the Initium bootloader with your kernel you need to add some code. To know how you do that, you need read the [Initium Boot Protocol](https://github.com/gil0mendes/Initium/blob/master/documentation/initium-protocol.md).

> Note: all example code is written in C

### 2.2 Compile

After you adapt your kernel, for been recognized by the Initium bootloader, you can now compile the Initium. The only thing you need to do is run the follow command on terminal:

```shell
# generate the configuration file
$ scons config

# compile Initium
$ scons
```

#### 2.2.1 Options

You can configure some Initium parameters and component you will be part of the final binary, that can be made with the scons `config` target:

```shell
$ scons config
```

### 2.2.2 Test

Initium comes with a test kernel and a set of script who allow developers test code changes in a very easy way.

```shell
# normal boot for the selected target
$ scons qemu

# test PXE
$ scons QEMU=pxe qemu
```
