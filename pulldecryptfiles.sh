#!/sbin/sh

# This pulls the files that are needed for decrypt
# allowing us to leave /vendor and /system unmounted

if [ "$(mount|grep "/dev/block/bootdevice/by-name/APP on /system ")" == "" ];then
	mount -o rw,remount rootfs /
	mkdir -p /system
	mkdir -p /tmp/system
	mount -t ext4 -o ro /dev/block/bootdevice/by-name/APP /system
	cp -r /system/lib64 /tmp/system/
	cp -r /system/system/lib64 /tmp/system/
	umount /system
	mv /tmp/system/* /system/
	rmdir /tmp/system
fi

if [ "$(mount|grep "/dev/block/bootdevice/by-name/VNR on /vendor")" == "" ];then
	mkdir -p /vendor
	mkdir -p /tmp/vendor
	mount -t ext4 -o ro /dev/block/bootdevice/by-name/VNR /vendor
	cp -r /vendor/lib64 /tmp/vendor/
	umount /vendor
	mv /tmp/vendor/* /vendor/
	rmdir /tmp/vendor
fi

setprop pulldecryptfiles.finished 1

exit 0

