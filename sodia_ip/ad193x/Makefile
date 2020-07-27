#KDIR ?= ../../software/linux-socfpga
KDIR ?= ../../linux-socfpga

default:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR)

clean:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) clean

help:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) help



#CROSS_COMPILE ?= /home/sanchayan/Toradex/gcc-linaro/bin/arm-linux-gnueabihf-
#ARCH          ?= arm
#SOURCE_DIR    ?= /home/weber/atlas_linux_ghrd/finished_software/KernelDrivers/FIR
 
#AS          = $(CROSS_COMPILE)as
#LD          = $(CROSS_COMPILE)ld
#CC          = $(CROSS_COMPILE)gcc
#CPP         = $(CC) -E
#AR          = $(CROSS_COMPILE)ar
#NM          = $(CROSS_COMPILE)nm
#STRIP       = $(CROSS_COMPILE)strip
#OBJCOPY     = $(CROSS_COMPILE)objcopy
#OBJDUMP     = $(CROSS_COMPILE)objdump
# 
#obj-m     += adc_test.o
#ccflags-y += -I$(SOURCE_DIR)/arch/arm
# 
#all:
#	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(SOURCE_DIR) M=$(PWD) modules
# 
#clean:
#	rm *.o *.ko *.symvers *.order
