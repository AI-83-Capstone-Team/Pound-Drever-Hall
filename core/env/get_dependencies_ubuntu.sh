echo
echo "Setup development packages"

# generic dependencies
sudo apt-get install make curl xz-utils -y
# U-Boot build dependencies
sudo apt-get install libssl-dev device-tree-compiler u-boot-tools -y
# secure chroot
sudo apt-get install schroot -y
# QEMU
sudo apt-get install qemu qemu-user qemu-user-static -y
# 32 bit libraries
sudo apt-get install lib32z1 lib32ncurses5 libbz2-1.0:i386 lib32stdc++6 -y

sudo apt-get install device-tree-compiler -y

sleep 1
echo
echo -n "Complete development packages "
print_ok

sleep 1

echo
echo "Setup Meson python packages"

sudo apt-get install python3 python3-pip -y
sudo pip3 install --upgrade pip -y
sudo pip3 install meson -y
sudo apt-get install ninja-build -y

sleep 1
echo
echo -n "Complete Meson python packages "
print_ok