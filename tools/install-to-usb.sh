sudo mkfs.vfat -F 32 -n DBOS -I $1
sudo mount $1 /mnt
sudo cp -r sysroot/* /mnt
sudo grub-install --target=i386-pc --root-directory=/mnt/ --no-floppy --recheck --force $1
