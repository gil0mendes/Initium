# Initium Readme

### Table of Contents

	1 Introduction
	2 Using Initium
		2.1 Configure the Kernel
		2.2 Build Initium
			2.2.1 Options

## 1 Introduction

Initium is a bootloader for the x86 and ARM processors. This bootloader supports the BIOS legacy PC boot system and the Extended Firmeware Interface(EFI). This bootloader was originaly created for the  [Infinity OS](https://github.com/gil0mendes/Infinity-OS)
project.
Click in the next link to see the [Initium Boot Protocol](https://github.com/gil0mendes/Initium/blob/master/documentation/initium-protocol.md).

Thanks for using Initium!

## 2 Using Initium

### 2.1 Configure

To use the Initium bootloader with your kernel you need to add some code. To know how you do that, you need read the [Initium Boot Protocol](https://github.com/gil0mendes/Initium/blob/master/documentation/initium-protocol.md).

### 2.2 Compile

After you adapt your kernel, for been recognised by the Initium bootloader, you can now compile the Initium.
Only you need to do is run the follow command on terminal:

	scons [options]

To see the building options you can run:

	scons -h

#### 2.2.1 Options

For now you have two possible configurations, `CONFIG` and `CROSS_COMPILE`. The `CROSS_COMPILE` option is optional and de `CONFIG` option is required for compiling.

The `CONFIG` options, is to select a target system. For now the only target system is the `bios` config.
