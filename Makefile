obj-m := myled.o

myled.ko: myled.c
	make -C /usr/src/linux M=$(PWD) modules

clean:
	make -C /usr/src/lunux M=$(PWD) clean
