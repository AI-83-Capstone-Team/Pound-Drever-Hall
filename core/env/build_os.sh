#!/bin/bash
# Minimal build driver for Red Pitaya Z10 / MASTER
# Run from core/env/

set -e

#TODO: add better logic for this in the future
export PROJECT="pdh_core"
export RTL_DIR="hw/"
export BUILD_DIR="build"

# --- config you may need to tweak ---
PATH_XILINX_SDK=/home/nolan/devtools/xilinx/vitis/Vitis/2020.1
PATH_XILINX_VIVADO=/home/nolan/devtools/xilinx/Vivado/2020.1
RP_UBUNTU=redpitaya_OS_12-38-21_10-Oct-2023.tar.gz   # rootfs tarball
SCHROOT_CONF_PATH=/etc/schroot/chroot.d/red-pitaya-ubuntu.conf

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"

print_ok()   { echo -e "\033[92m[OK]\033[0m"; }
print_fail() { echo -e "\033[91m[FAIL]\033[0m"; }

echo "Start build process..."
sleep 1

# gmake shim (harmless if it already exists)
if ! command -v gmake >/dev/null 2>&1; then
  sudo ln -sf /usr/bin/make /usr/bin/gmake || true
fi

# --- check Xilinx paths ---
if [[ -d "$PATH_XILINX_SDK" ]]; then
  echo -n "$PATH_XILINX_SDK exists on your filesystem. "
  print_ok
else
  echo -n "Can't find $PATH_XILINX_SDK on your PC. "; print_fail
  echo "Fix PATH_XILINX_SDK in build_os.sh"
  exit 1
fi

sleep 1

if [[ -d "$PATH_XILINX_VIVADO" ]]; then
  echo -n "$PATH_XILINX_VIVADO exists on your filesystem. "
  print_ok
else
  echo -n "Can't find $PATH_XILINX_VIVADO on your PC. "; print_fail
  echo "Fix PATH_XILINX_VIVADO in build_os.sh"
  exit 1
fi

sleep 1

# --- download RP Ubuntu rootfs tarball (for schroot) ---
export DL="${ROOT_DIR}/tmp/DL"
mkdir -p "$DL"
echo -n "Created directory for download at $DL "
[[ -d "$DL" ]] && print_ok || { print_fail; exit 1; }

if [[ -z "${1:-}" ]]; then
  echo -n "Download Red Pitaya Ubuntu OS tarball... "
  cd "$DL"
  wget -N "http://downloads.redpitaya.com/downloads/LinuxOS/$RP_UBUNTU"
else
  echo "Using Ubuntu OS tarball from parameter: $1"
  RP_UBUNTU="$1"
  cp -f "$RP_UBUNTU" "$DL/$RP_UBUNTU"
  cd "$DL"
fi

echo -n "Check Red Pitaya Ubuntu OS tarball... "
if [[ -f "$RP_UBUNTU" ]]; then
  sudo chown root:root "$RP_UBUNTU"
  sudo chmod 664 "$RP_UBUNTU"
  print_ok
else
  print_fail
  exit 1
fi

cd "$ROOT_DIR"

# --- write schroot config pointing at that tarball ---
if [[ -f "$SCHROOT_CONF_PATH" ]]; then
  echo "File $SCHROOT_CONF_PATH exists, removing..."
  sudo rm -f "$SCHROOT_CONF_PATH"
fi

echo "Write new schroot configuration"
{
  echo "[red-pitaya-ubuntu]"
  echo "description=Red Pitaya Ubuntu rootfs"
  echo "type=file"
  echo "file=$DL/$RP_UBUNTU"
  echo "users=root"
  echo "root-users=root"
  echo "root-groups=root"
  echo "personality=linux"
  echo "preserve-environment=true"
} | sudo tee "$SCHROOT_CONF_PATH" >/dev/null

echo -n "Complete write new configuration "
print_ok
echo

# --- toolchain / env for x86.mak ---
export ENABLE_LICENSING=0
export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm
export PATH="$PATH:$PATH_XILINX_VIVADO/bin"
export PATH="$PATH:$PATH_XILINX_SDK/bin"
export PATH="$PATH:$PATH_XILINX_SDK/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/"

ENABLE_PRODUCTION_TEST=0
GIT_COMMIT_SHORT="$(git rev-parse --short HEAD 2>/dev/null || echo local)"

# --- 1) build FPGA / FSBL / PL DTS on host (Z10, MASTER) ---
make -f x86.mak fpga MODEL=Z10 STREAMING=MASTER

# --- 2) build userland / filesystem bits inside ARM rootfs (if needed) ---
schroot -c red-pitaya-ubuntu <<-EOL_CHROOT
  cd /   # adjust if your in-chroot Makefile lives elsewhere
  make -f Makefile CROSS_COMPILE="" \\
       REVISION=$GIT_COMMIT_SHORT ENABLE_PRODUCTION_TEST=0 \\
       ENABLE_LICENSING=0 BUILD_NUMBER=1
EOL_CHROOT

# --- 3) build U-Boot, Linux, boot.bin, zip on host ---
make -f x86.mak all zip MODEL=Z10 STREAMING=MASTER

echo "Done."
