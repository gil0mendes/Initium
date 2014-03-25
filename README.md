# LAOS Readme

### Table of Contents

	1 Introduction
	2 Using LAOS
		2.1 Configure the Kernel
		2.2 Build LAOS
			2.2.1 Options

## 1 Introduction

LAOS is a bootloader for the x86 and ARM processors. This bootloader supports the BIOS legacy PC boot system
and the Extended Firmeware Interface(EFI). This bootloader was originaly created for the  [Infinity OS](https://github.com/gil0mendes/Infinity-OS)
project.
Click in the next link to see the [LAOS Boot Protocol](https://github.com/gil0mendes/LAOS/blob/master/documentation/laos-protocol.md).

Thanks for using LAOS!

Gil Mendes

## 2 Using LAOS

### 2.1 Configure

To use the LAOS bootloader with your kernel you need to add some code. To know how you do that, you need read
the [LAOS Boot Protocol](https://github.com/gil0mendes/LAOS/blob/master/documentation/laos-protocol.md).

### 2.2 Compile

After you adapt your kernel, for been recognised by the LAOS bootloader, you can now compile the LAOS. 
Only you need to do is run the follow command on terminal:

	scons [options]
	
To see the buiding options you can run:

	scons -h

#### 2.2.1 Options

For now you have to possible configurations, `CONFIG` and `CROSS_COMPILE`. The `CROSS_COMPILE` option is 
optional and de `CONFIG` option is required for compiling.

The `CONFIG` options, is to select a target system. For now the only target system is the `bios`config.

**In the near feature we will add some other targets like EFI.**
