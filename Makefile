CONFIG_MODULE_SIG=n

CC = gcc
# INC=-I/usr/include
obj-m := pcilatdriver.o
pcilatdriver-objs := src/driver.o src/tsc.o src/int.o src/pcireg.o src/drvcfg.o src/iomem.o src/chrdev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
