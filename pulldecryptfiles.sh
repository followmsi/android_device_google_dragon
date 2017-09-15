#!/sbin/sh

# This pulls the files that are needed for decrypt
# allowing us to leave /vendor and /system unmounted

if [ "$(mount|grep "/dev/block/bootdevice/by-name/APP on /system ")" == "" ];then
        mount -o rw,remount rootfs /
	mkdir /system
        mount -t ext4 -o ro /dev/block/bootdevice/by-name/APP /system
	mkdir /tmp/system
	cp -r /system/lib64 /tmp/system/
	cp -r /system/bin /tmp/system/
	umount /system
	mkdir /system/lib64
	mkdir /system/bin
	mv /tmp/system/lib64/* /system/lib64/
	mv /tmp/system/bin/* /system/bin/
	rmdir /tmp/system/lib64 /tmp/system/bin /tmp/system
fi

if [ "$(mount|grep "/dev/block/bootdevice/by-name/VNR on /vendor")" == "" ];then
	mkdir -p /vendor
	mkdir -p /tmp/vendor
        mount -t ext4 -o ro /dev/block/bootdevice/by-name/VNR /vendor

	cp -r /vendor/lib64 /tmp/vendor/
	cp -r /vendor/bin /tmp/vendor/

	umount /vendor

	mv /tmp/vendor/* /vendor/
	rmdir /tmp/vendor
fi

if [ "$(mount|grep "/dev/block/bootdevice/by-name/UDA on /data")" == "" ];then
	LD_LIBRARY_PATH=/system/lib64 /system/bin/e2fsck  -y -E journal_only /dev/block/bootdevice/by-name/userdata
        LD_LIBRARY_PATH=/system/lib64 /system/bin/tune2fs -Q ^usrquota,^grpquota,^prjquota /dev/block/bootdevice/by-name/userdata
        mount /dev/block/bootdevice/by-name/UDA /data
fi


setprop pulldecryptfiles.finished 1
exit 0

