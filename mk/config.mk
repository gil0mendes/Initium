# Current directory
ROOT=$(PWD)

# LD variables
LDFLAGS = -nostdlib -znocombreloc -shared -Bsymbolic --no-undefined
LD = $(prefix)ld.lld
OBJCOPY = $(prefix)objcopy
