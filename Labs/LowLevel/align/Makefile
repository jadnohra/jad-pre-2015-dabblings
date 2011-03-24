                            

CELL_MK_DIR ?= $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

PPU_SRCS = align.ppu.c
PPU_TARGET = align.ppu.elf
PPU_LDLIBS   = -llv2dbg_stub -lsysmodule_stub

include $(CELL_MK_DIR)/sdk.target.mk


