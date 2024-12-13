#!/bin/bash

DEV_ID=$1
DEV_BUS="0000"

if [ -f /sys/bus/pci/devices/${DEV_BUS}:${DEV_ID}/vendor ]; then
    DEV_VENDOR=$(input=`cat /sys/bus/pci/devices/${DEV_BUS}:${DEV_ID}/vendor`;echo ${input#"0x"})
    DEV_DEVICE=$(input=`cat /sys/bus/pci/devices/${DEV_BUS}:${DEV_ID}/device`;echo ${input#"0x"})
else
    echo "Device $DEV_ID does not appear to be valid!"
    exit 1
fi

DEV_DRIVER=$(target=`readlink -f /sys/bus/pci/devices/${DEV_BUS}:${DEV_ID}/driver`;target2=`basename $target`;echo $target2)
echo "${DEV_ID}"
echo "Vendor ID: ${DEV_VENDOR}"
echo "Device ID: ${DEV_DEVICE}"
echo "Driver:    ${DEV_DRIVER}"
echo ""

if [ -f /sys/bus/pci/devices/${DEV_BUS}:${DEV_ID}/driver/unbind ]; then
    echo "Unregistering device from current driver..."
    echo ${DEV_BUS}:${DEV_ID} >/sys/bus/pci/devices/${DEV_BUS}:${DEV_ID}/driver/unbind
else
    echo "Device not currently registered to driver. Will bind it."
fi

echo "Loading pcilatdriver with claim on ${DEV_VENDOR}:${DEV_DEVICE}"

/sbin/insmod ../pcilatdriver.ko device=${DEV_VENDOR}:${DEV_DEVICE}

echo "Complete!"

