# See LICENSE.SiFive for license details

boot_rom = boot_rom.sv

GCC?=$(RISCV)/bin/riscv32-unknown-elf-gcc
OBJCOPY?=$(RISCV)/bin/riscv32-unknown-elf-objcopy
OBJDUMP?=$(RISCV)/bin/riscv32-unknown-elf-objdump
PYTHON?=python

all: $(boot_rom)

%.sv: %.img
	$(PYTHON) gen_rom.py $<

%.img: %.bin
	dd if=$< of=$@ bs=256 count=1

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.elf: $(findstring boot_rom, $(boot_rom)).S link.ld
	$(GCC) -I../../../sw/device/lib/runtime/ -I../../../sw/device/lib/drivers/ -Tlink.ld $< -nostdlib -fPIC -static -Wl,--no-gc-sections -o $@

%.dump: %.elf
	$(OBJDUMP) -d $< --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data  > $@

clean:
	rm -f *.img *.bin *.sv
