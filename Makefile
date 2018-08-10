obj-m := myled_cls.o

myled_cls.ko: myled_cls.c
	make -C /usr/src/linux M=$(PWD) modules

clean:
	rm -f *.o *.order *.mod.c *.symvers *.ko
	rm -rf .*.cmd .tmp_versions
