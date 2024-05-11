# Simple Makfile for echodev-drv.c driver.

obj-m += echodev-drv.o

all:
	make -C ../linux-6.1.90 M=$(PWD) modules

clean:
	make -C ../linux-6.1.90 M=$(PWD) clean



