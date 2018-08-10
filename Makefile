<<<<<<< HEAD
obj-m := myled_cls.o

myled_cls.ko: myled_cls.c
	make -C /usr/src/linux M=$(PWD) modules

clean:
	rm -f *.o *.order *.mod.c *.symvers *.ko
	rm -rf .*.cmd .tmp_versions
=======
obj-m := myled.o

myled.ko: myled.c
	make -C /usr/src/linux M=$(PWD) modules

clean:
	make -C /usr/src/lunux M=$(PWD) clean
>>>>>>> c216ea5fa826bb883cdddf8d809ce424f04451ef
