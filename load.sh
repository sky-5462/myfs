sudo insmod myfs.ko
sudo losetup /dev/loop0 image
sudo mount -t myfs /dev/loop0 /mnt/myfs/