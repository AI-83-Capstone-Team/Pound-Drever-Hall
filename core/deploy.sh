#!/bin/bash

# Configuration
HOST="10.42.0.62"
USER="root"
PASS="root"

SRC1="hw/build/boot.bin"
DEST1="/root/boot.bin"

SRC2="sw/"
DEST2="/root/sw"

# Define the sshpass prefix to stay DRY (Don't Repeat Yourself)
PASS_CMD="sshpass -p $PASS"
SSH_OPTS="-o StrictHostKeyChecking=no"

echo "🚀 Starting Deployment to $HOST..."

# 1. Remove old files
echo "🧹 Cleaning target..."
$PASS_CMD ssh $SSH_OPTS $USER@$HOST "rm -f $DEST1; rm -rf $DEST2"

# 2. Transfer Hardware Bin
echo "📦 Transferring boot.bin..."
$PASS_CMD scp $SSH_OPTS "$SRC1" "$USER@$HOST:$DEST1"

# 3. Transfer Software Directory
echo "📂 Transferring sw directory..."
$PASS_CMD scp $SSH_OPTS -r "$SRC2" "$USER@$HOST:$DEST2"

# 4. Run the remote logic (FPGA config + Build + Run)
echo "🛠️  Configuring FPGA and Building Software..."
$PASS_CMD ssh $SSH_OPTS $USER@$HOST "
    echo '--> Programming FPGA...';
    fpgautil -b $DEST1;
    
    echo '--> Building Server...';
    cd $DEST2 && make clean && make server;
    pkill -9 server 
    echo '🚀 Starting Server...';
    ./build/server;

"
