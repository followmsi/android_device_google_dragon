#!/vendor/bin/sh

# Copyright (c) 2015-2018, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

bt_dtb_file="/proc/device-tree/chosen/nvidia,bt-mac"
bt_fct_file="/mnt/vendor/factory/bluetooth/bt_mac.txt"

# Deleted this line when nvidia,bt-mac becomes irrelevant and change
# bt_dtb_file above to /proc/device-tree/chosen/nvidia,bluetooth-mac
[ -e $bt_dtb_file ] || bt_dtb_file="/proc/device-tree/chosen/nvidia,bluetooth-mac"

if [ -e $bt_dtb_file ] ; then

    cat $bt_dtb_file | toybox_vendor grep "[0-9A-Fa-f][0-9A-Fa-f]:[0-9A-Fa-f][0-9A-Fa-f]:[0-9A-Fa-f][0-9A-Fa-f]:[0-9A-Fa-f][0-9A-Fa-f]:[0-9A-Fa-f][0-9A-Fa-f]:[0-9A-Fa-f][0-9A-Fa-f]"

    # Check if the contents of device-tree/nct/bt represent a correct mac address
    if [ $? -eq 0 ] ; then
        setprop ro.vendor.bt.bdaddr_path "${bt_dtb_file}"
    else
        setprop ro.vendor.bt.bdaddr_path "${bt_fct_file}"
    fi
else
    setprop ro.vendor.bt.bdaddr_path "${bt_fct_file}"
fi
