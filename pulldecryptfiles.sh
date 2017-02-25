#!/sbin/sh

# This pulls the files out of /vendor that are needed for decrypt
# This allows us to decrypt the device in recovery and still be
# able to unmount /vendor when we are done.

mkdir -p /vendor
mount -t ext4 -o ro /dev/block/platform/700b0600.sdhci/by-name/VNR /vendor

mkdir -p /system
mount -t ext4 -o ro /dev/block/platform/700b0600.sdhci/by-name/APP /system


cp /vendor/lib/hw/keystore.dragon.so /sbin/keystore.dragon.so.32
cp /vendor/lib64/hw/keystore.dragon.so /sbin/keystore.dragon.so


cp /system/lib/libkeymaster1.so /sbin/libkeymaster1.so.32
cp /system/lib64/libkeymaster1.so /sbin/libkeymaster1.so

cp /system/lib/libkeymaster_messages.so /sbin/libkeymaster_messages.so.32
cp /system/lib64/libkeymaster_messages.so /sbin/libkeymaster_messages.so

cp /system/lib/libkeystore_binder.so /sbin/libkeystore_binder.so.32
cp /system/lib64/libkeystore_binder.so /sbin/libkeystore_binder.so

cp /system/lib/libkeystore-engine.so /sbin/libkeystore-engine.so.32
cp /system/lib64/libkeystore-engine.so /sbin/libkeystore-engine.so


umount /vendor
umount /system


mkdir -p /vendor/lib/hw
mkdir -p /vendor/lib64/hw

cp /sbin/keystore.dragon.so.32 /vendor/lib/hw/keystore.dragon.so
cp /sbin/keystore.dragon.so /vendor/lib64/hw/keystore.dragon.so


mkdir -p /system/lib
mkdir -p /system/lib64


mv /sbin/libkeymaster1.so.32 /system/lib/libkeymaster1.so
cp /sbin/libkeymaster1.so /system/lib64/libkeymaster1.so

mv /sbin/libkeymaster_messages.so.32 /system/lib/libkeymaster_messages.so
cp /sbin/libkeymaster_messages.so /system/lib64/libkeymaster_messages.so

mv /sbin/libkeystore_binder.so.32 /system/lib/libkeystore_binder.so
cp /sbin/libkeystore_binder.so /system/lib64/libkeystore_binder.so

mv /sbin/libkeystore-engine.so.32 /system/lib/libkeystore-engine.so
cp /sbin/libkeystore-engine.so /system/lib64/libkeystore-engine.so


