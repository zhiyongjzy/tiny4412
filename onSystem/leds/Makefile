obj-m := ldm.o
$(obj-m:.o=)-objs := led.o

KERN_ROOT := /home/jzy/Documents/tiny4412/4412_res/linux-3.5
TFTP_BOOT := ~/Public

ccflags-y += -Wno-declaration-after-statement

all: build mv clean

build:
	make M=`pwd` -C $(KERN_ROOT) modules

mv:
	mv $(obj-m:.o=.ko) $(TFTP_BOOT)

clean:
	make M=`pwd` -C $(KERN_ROOT) clean

.PHONY: all build mv clean
