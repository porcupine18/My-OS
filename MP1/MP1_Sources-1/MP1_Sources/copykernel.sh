sudo mount -o loop mp1.img /mnt/floppy
sudo cp kernel.elf /mnt/floppy/
sleep 1s
sudo umount /mnt/floppy/
