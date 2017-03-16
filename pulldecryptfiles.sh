#!/sbin/sh

# This pulls the files out of /vendor that are needed for decrypt
# This allows us to decrypt the device in recovery and still be
# able to unmount /vendor when we are done.

mkdir -p /vendor
mount -t ext4 -o ro /dev/block/platform/700b0600.sdhci/by-name/VNR /vendor

mkdir -p /system
mount -t ext4 -o ro /dev/block/platform/700b0600.sdhci/by-name/APP /system


cp /vendor/lib64/hw/gatekeeper.dragon.so /sbin/gatekeeper.dragon.so
cp /vendor/lib64/hw/keystore.dragon.so /sbin/keystore.dragon.so

cp /system/lib64/libgatekeeper.so /sbin/libgatekeeper.so
cp /system/lib64/libkeymaster1.so /sbin/libkeymaster1.so
cp /system/lib64/libkeymaster_messages.so /sbin/libkeymaster_messages.so
cp /system/lib64/libkeystore_binder.so /sbin/libkeystore_binder.so
cp /system/lib64/libkeystore-engine.so /sbin/libkeystore-engine.so


umount /vendor
umount /system


mkdir -p /vendor/lib64/hw
mkdir -p /system/lib64


cp /sbin/gatekeeper.dragon.so /vendor/lib64/hw/gatekeeper.dragon.so
cp /sbin/keystore.dragon.so /vendor/lib64/hw/keystore.dragon.so

cp /sbin/libgatekeeper.so /system/lib64/libgatekeeper.so
cp /sbin/libkeymaster1.so /system/lib64/libkeymaster1.so
cp /sbin/libkeymaster_messages.so /system/lib64/libkeymaster_messages.so
cp /sbin/libkeystore_binder.so /system/lib64/libkeystore_binder.so
cp /sbin/libkeystore-engine.so /system/lib64/libkeystore-engine.so

