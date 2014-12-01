obj-m := ldm.o
$(obj-m:.o=)-objs := fb.o

KERN_ROOT := /home/jzy/Documents/tiny4412/4412_res/linux-3.5
TFTP_BOOT := ~/Public
NFS_ROOT := /home/jzy/Documents/tiny4412/4412_res/nfsroot

ccflags-y += -Wno-declaration-after-statement

all: build mv clean

build:
	make M=`pwd` -C $(KERN_ROOT) modules

mv:
	mv $(obj-m:.o=.ko) $(NFS_ROOT)

clean:
	make M=`pwd` -C $(KERN_ROOT) clean

.PHONY: all build mv clean
