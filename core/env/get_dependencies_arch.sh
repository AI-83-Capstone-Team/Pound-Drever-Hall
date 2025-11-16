#Assume valid python shell is aready installed
echo
echo "Setup development packages"

# generic dependencies
sudo pacman -S --needed --noconfirm make curl xz

# U-Boot build dependencies
sudo pacman -S --needed --noconfirm openssl dtc uboot-tools

# secure chroot (AUR)
# requires yay (or another AUR helper)
yay -S --needed --noconfirm schroot

# QEMU
sudo pacman -S --needed --noconfirm qemu qemu-user qemu-user-static

# 32-bit libraries (requires multilib enabled in /etc/pacman.conf)
# If these don't install properly:
#   1. sudo vim /etc/pacman.conf
#   2. Uncomment:
#       #[multilib]
#       #Include = /etc/pacman.d/mirrorlist
sudo pacman -S --needed --noconfirm lib32-zlib lib32-ncurses lib32-bzip2 lib32-gcc-libs

sudo pacman -S --needed --noconfirm xorg-xlsclients

# device tree compiler (already installed above as 'dtc'
# sudo pacman -S --needed --noconfirm dtc

sleep 1
echo
echo -n "Complete development packages "
print_ok

sleep 1

echo
echo "Setup Meson python packages"

# Python + pip + Meson + Ninja from repos (cleaner than pip for these)
sudo pacman -S --needed --noconfirm meson ninja

# If you really want Meson via pip instead of the repo version, you'd do:
# sudo pip3 install --upgrade pip
# sudo pip3 install meson

sleep 1
echo
echo -n "Complete Meson python packages "
