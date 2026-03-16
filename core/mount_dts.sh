
dtc -@ -I dts -O dtb -o /root/pdh_irq.dtbo /root/pdh_irq.dts 2>/dev/null
[ -d /sys/kernel/config/device-tree ] || mount -t configfs none /sys/kernel/config
rmdir /sys/kernel/config/device-tree/overlays/pdh_irq 2>/dev/null || true
rmdir /sys/kernel/config/device-tree/overlays/Full    2>/dev/null || true
mkdir -p /sys/kernel/config/device-tree/overlays/pdh_irq
cp /root/pdh_irq.dtbo /sys/kernel/config/device-tree/overlays/pdh_irq/dtbo
sleep 0.5
ls /dev/uio/pdh_uio && echo '--> /dev/uio/pdh_uio ready.' || echo 'WARNING: /dev/uio/pdh_uio not found!'
