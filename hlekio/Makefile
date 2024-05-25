IGNORE := $(shell bash -c "source ../env.sh; /usr/bin/env | sed 's/=/:=/' | sed 's/^/export /' > makeenv")                         
include makeenv

obj-m := hlekio.o
hlekio-y := hlekio_common.o hlekio_init.o hlekio_probe.o hlekio_common_fops.o hlekio_in_fops.o hlekio_in_isr.o hlekio_out_fops.o
KERNEL_DIR ?= /mnt/SHARE/home-lab-easy-kit/hlekio/linux
DEBUG_CFLAGS += -g -O0 -DDEBUG

all:
	make -C $(KERNEL_DIR) ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_CROSS_COMPILER} M=$(PWD) C=1 modules

alldebug:
	@echo "+++ BUILDING DEBUG +++"
	make -C $(KERNEL_DIR) ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_CROSS_COMPILER} M=$(PWD) C=1 modules EXTRA_CFLAGS="$(DEBUG_CFLAGS)" 

clean:
	make -C $(KERNEL_DIR) ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_CROSS_COMPILER} M=$(PWD) clean
	rm -rf makeenv

deploy:
#	scp *.ko pi@192.168.1.93:/home/pi

