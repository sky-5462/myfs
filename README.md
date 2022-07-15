# myfs

For learning，边看边搓，对照来看

基于Debian 11，内核版本5.10.127

## Compile

make / make clean 就行

## Test

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

## Reference

- https://github.com/psankar/simplefs
- https://docs.kernel.org/filesystems/vfs.html
- https://elixir.bootlin.com/linux/v5.10.127/source/include/linux/fs.h
