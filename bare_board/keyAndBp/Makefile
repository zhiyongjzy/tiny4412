TGT := keyAndBp.bin
OBJS := start.o keyAndBp.o 
CROSS_COMPILER := arm-linux-
CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld
OBJCOPY := $(CROSS_COMPILER)objcopy
OBJDUMP := $(CROSS_COMPILER)objdump
CFLAGS := -Wall -marm -ffreestanding -nostdlib
LDFLAGS := -T$(TGT:.bin=.lds) -N
RM := -rm -f
export BL2 := $(PWD)/$(TGT)

$(TGT):$(OBJS)
	$(LD) $(LDFLAGS) -o $(TGT:.bin=.elf) $^
	$(OBJCOPY) -O binary -S $(TGT:.bin=.elf) $@
%.o:%.c
	$(CC) $(CFLAGS) -c -O2 -o $@ $<
%.o:%.S
	$(CC) $(CFLAGS) -c -O2 -o $@ $<

load:
	make -C ../tools
dump:$(TGT)
	$(OBJDUMP) -D $(TGT:.bin=.elf)


clean:
	$(RM) $(OBJS) $(TGT) $(TGT:.bin=.elf)

.PHONY: clean load
