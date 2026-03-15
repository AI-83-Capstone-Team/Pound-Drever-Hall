#!/bin/bash

# Configuration
HOST="10.42.0.62"
USER="root"
PASS="root"

SRC1="hw/build/boot.bin"
DEST1="/root/boot.bin"

SRC2="sw/"
DEST2="/root/sw"

SRC3="dts/pdh_irq.dts"
DEST3="/root/pdh_irq.dts"

# Define the sshpass prefix to stay DRY (Don't Repeat Yourself)
PASS_CMD="sshpass -p $PASS"
SSH_OPTS="-o StrictHostKeyChecking=no"

echo "🚀 Starting Deployment to $HOST..."

# 1. Remove old files
echo "🧹 Cleaning target..."
$PASS_CMD ssh $SSH_OPTS $USER@$HOST "rm -f $DEST1; rm -rf $DEST2; rm -f /root/pdh_irq.dts /root/pdh_irq.dtbo"

# 2. Transfer Hardware Bin
echo "📦 Transferring boot.bin..."
$PASS_CMD scp $SSH_OPTS "$SRC1" "$USER@$HOST:$DEST1"

# 3. Transfer Software Directory
echo "📂 Transferring sw directory..."
$PASS_CMD scp $SSH_OPTS -r "$SRC2" "$USER@$HOST:$DEST2"

# 4. Transfer DT overlay source
echo "🌲 Transferring DT overlay..."
$PASS_CMD scp $SSH_OPTS "$SRC3" "$USER@$HOST:$DEST3"

# 5. Run the remote logic (FPGA config + DT overlay + Build + Run)
echo "🛠️  Configuring FPGA and Building Software..."
$PASS_CMD ssh $SSH_OPTS $USER@$HOST "
    echo '--> Programming FPGA...';
    fpgautil -b boot.bin;

    echo '--> Loading UIO interrupt overlay...';
    dtc -@ -I dts -O dtb -o /root/pdh_irq.dtbo /root/pdh_irq.dts 2>/dev/null;
    [ -d /sys/kernel/config/device-tree ] || mount -t configfs none /sys/kernel/config;
    rmdir /sys/kernel/config/device-tree/overlays/pdh_irq 2>/dev/null || true;
    rmdir /sys/kernel/config/device-tree/overlays/Full    2>/dev/null || true;
    mkdir -p /sys/kernel/config/device-tree/overlays/pdh_irq;
    cp /root/pdh_irq.dtbo /sys/kernel/config/device-tree/overlays/pdh_irq/dtbo;
    sleep 0.5;
    ls /dev/uio/pdh_uio && echo '--> /dev/uio/pdh_uio ready.' || echo 'WARNING: /dev/uio/pdh_uio not found!';

    echo '--> Building Server...';
    cd $DEST2 && make clean && make server;
    pkill -9 server;
    echo '🚀 Starting Server...';
    ./build/server;

"
