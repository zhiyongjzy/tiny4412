TGT := boot.bin
SUBDIR := lib
export SUBTGT :=  build-in.o

OBJS := start.o clk.o uart.o string.o i2c.o eeprom.o

export CROSS_COMPILER := arm-linux-
export CC := $(CROSS_COMPILER)gcc
export LD := $(CROSS_COMPILER)ld
export OBJCOPY := $(CROSS_COMPILER)objcopy
export OBJDUMP := $(CROSS_COMPILER)objdump
export AR := $(CROSS_COMPILER)ar

export RM := -rm -f

CFLAGS := -marm -ffreestanding -nostdlib -I./include
LDFLAGS := -Tboot.lds -N

export BL2 := $(PWD)/$(TGT)

$(TGT): $(SUBDIR:=/$(SUBTGT)) $(OBJS)
	$(LD) $(LDFLAGS) -o $(TGT:.bin=.elf) $(OBJS) $(SUBDIR:=/$(SUBTGT))
	$(OBJCOPY) -O binary -S $(TGT:.bin=.elf) $@

$(SUBDIR:=/$(SUBTGT)) :
	make -C $(SUBDIR)
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o:%.S
	$(CC) $(CFLAGS) -c -o $@ $<


load:$(TGT)
	make -C ../tools
dump:$(TGT)
	$(OBJDUMP) -D $(TGT:.bin=.elf)


clean:
	$(RM) $(OBJS) $(TGT) $(TGT:.bin=.elf)
	for dir in $(SUBDIR); do \
		make -C $$dir clean; \
	done

.PHONY: clean load 
