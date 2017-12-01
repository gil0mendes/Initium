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
LDFLAGS = -nostdlib -znocombreloc -shared -Bsymbolic --no-undefined
LD = ld.lld
OBJCOPY = $(prefix)objcopy
