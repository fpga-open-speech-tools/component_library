KDIR ?= /home/harthan/FPGA_Projects/linux-socfpga
default:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR)

clean:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) clean

help:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) help
