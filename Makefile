rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

PATH := $(PATH):$(DEVKITARM)/bin

CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OC := arm-none-eabi-objcopy

name := Corbenik

# If unset, the primary folder is /corbenik.
fw_folder ?= corbenik

dir_source := source
dir_data   := data
dir_build  := build
dir_out    := out

REVISION := r$(shell git rev-list --count HEAD):$(shell git rev-parse HEAD | head -c8)

ASFLAGS := -mlittle-endian -mcpu=arm946e-s -march=armv5te
CFLAGS  := -MMD -MP -Wall -Wextra -Werror -fno-omit-frame-pointer -Os $(ASFLAGS) -fno-builtin -std=c11 -DVERSION=\"$(REVISION)\"
FLAGS   := dir_out=$(abspath $(dir_out)) --no-print-directory
LDFLAGS := -nostdlib -Wl,-z,defs -lgcc -Wl,-Map,$(dir_build)/link.map

objects_cfw = $(patsubst $(dir_source)/%.s, $(dir_build)/%.o, \
			  $(patsubst $(dir_source)/%.c, $(dir_build)/%.o, \
			  $(call rwildcard, $(dir_source), *.s *.c)))

.PHONY: all
all: a9lh external

.PHONY: full
full: host/langemu.conf all

.PHONY: external
external:
	make -C external

.PHONY: a9lh
a9lh: $(dir_out)/arm9loaderhax.bin

.PHONY: reformat
reformat:
	clang-format -i $(dir_source)/*.{c,h} $(dir_source)/*/*.{c,h} external/loader/source/*.{c,h} external/loader/source/*/*.{c,h} host/*.c

host/langemu.conf:
	echo "Generating langemu.conf - may take a bit"
	cd host && ./generate_langemu_conf.sh
	mkdir -p $(dir_out)/corbenik/etc
	cp host/langemu.conf $(dir_out)/corbenik/etc/langemu.conf

.PHONY: clean
clean:
	rm -f host/langemu.conf
	make -C external clean
	rm -rf $(dir_out) $(dir_build)

.PHONY: $(dir_out)/arm9loaderhax.bin
$(dir_out)/arm9loaderhax.bin: $(dir_build)/main.bin
	@mkdir -p "$(dir_out)"
	@cp -av $< $@

$(dir_build)/main.bin: $(dir_build)/main.elf
	$(OC) -S -O binary $< $@

$(dir_build)/main.elf: $(objects_cfw)
	$(CC) -T linker.ld $(OUTPUT_OPTION) $^ $(LDFLAGS)

$(dir_build)/%.o: $(dir_source)/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(dir_build)/%.o: $(dir_source)/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<

$(dir_build)/fatfs/%.o: $(dir_source)/fatfs/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/fatfs/%.o: $(dir_source)/fatfs/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<

$(dir_build)/std/%.o: $(dir_source)/std/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/std/%.o: $(dir_source)/std/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<

$(dir_build)/firm/%.o: $(dir_source)/firm/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/firm/%.o: $(dir_source)/firm/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<

$(dir_build)/patch/%.o: $(dir_source)/patch/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/patch/%.o: $(dir_source)/patch/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<

include $(call rwildcard, $(dir_build), *.d)
