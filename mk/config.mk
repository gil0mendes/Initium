# Current directory
ROOT=$(PWD)

# Per host variables
UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
	VBM="/Applications/VirtualBox.app/Contents/MacOS/VBoxManage"
else
	VBM=VBoxManager
endif

# LD variables
LDFLAGS = -nostdlib -znocombreloc -shared -Bsymbolic --no-undefined --gc-sections
LD = ld.lld
CC = cc
OBJCOPY = $(prefix)objcopy

# Rust related configurations
export RUST_TARGET_PATH=$(abspath targets)
