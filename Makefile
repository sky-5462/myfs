obj-m += myfs.o
myfs-y += super.o dir.o file.o inode.o namei.o symlink.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
