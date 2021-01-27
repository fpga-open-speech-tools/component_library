KDIR ?= ~/linux-socfpga
default: driver overlay
driver:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) CROSS_COMPILE=arm-linux-gnueabihf-
overlay:
	dtc -I dts -O dtb -o dma.dtbo dma.dts
clean:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) clean
help:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) help
