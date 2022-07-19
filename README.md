# myfs

For learning，边看边搓，对照来看

基于Debian 11，内核版本5.10.127

## Compile

make / make clean 就行

## 注册文件系统

load.sh / unload.sh 脚本，可以看dmesg的消息

```bash
sky-5462@debian:~/myfs$ sudo dmesg | tail
[27829.589229] myfs: loading out-of-tree module taints kernel.
[27829.589258] myfs: module verification failed: signature and/or required key missing - tainting kernel
[27829.590146] Init myfs
[28043.634000] Exit myfs
```

加入register_filesystem，确认

```bash
sky-5462@debian:~/myfs$ ./load.sh
sky-5462@debian:~/myfs$ cat /proc/filesystems
# ...
nodev   myfs
```

## 挂载虚拟块设备

可以将文件挂载为虚拟块设备，让我们的文件系统更真实点..

```bash
sky-5462@debian:~/myfs$ dd if=/dev/zero of=image bs=4K count=262144
sky-5462@debian:~/myfs$ sudo losetup /dev/loop0 image
sky-5462@debian:~/myfs$ sudo mount -t myfs /dev/loop0 /mnt/myfs
```

取消挂载

```bash
sky-5462@debian:~/myfs$ sudo umount /mnt/myfs
sky-5462@debian:~/myfs$ sudo losetup -d /dev/loop0
```

改了下代码，让挂载时返回根的inode，现在可以成功挂载，可以看到目录大小和时间变了

```bash
sky-5462@debian:~/myfs$ ls -lh /mnt/
总用量 0
drwxr--r-- 1 root root 0  7月 18 10:00 myfs
```

当然由于还没实现具体函数，根目录里面是访问不了的

```bash
sky-5462@debian:~/myfs$ ls -lh /mnt/myfs/
ls: 无法访问 '/mnt/myfs/': 不是目录
```

## 格式化

定义了super_block和inode的数据结构，并格式化镜像文件

```bash
sky-5462@debian:~/myfs$ gcc myfs_format.c
sky-5462@debian:~/myfs$ ./a.out image
```

## Reference

- https://github.com/psankar/simplefs
- https://docs.kernel.org/filesystems/vfs.html
- https://elixir.bootlin.com/linux/v5.10.127/source/include/linux/fs.h
