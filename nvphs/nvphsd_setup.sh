#!/vendor/bin/sh

# Copyright (c) 2013-2018, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

# loop through thermal zones
for tz in /sys/class/thermal/thermal_zone*
do
    if [ -e "${tz}" ]; then
        chown system "${tz}/type" 2> /dev/null
        chmod 0444 "${tz}/type" 2> /dev/null
        chown system "${tz}/temp" 2> /dev/null
        chmod 0444 "${tz}/temp" 2> /dev/null
        chown system "${tz}"/trip_point_*_temp 2> /dev/null
        chmod 0664 "${tz}"/trip_point_*_temp 2> /dev/null
        chown system "${tz}"/trip_point_*_hyst 2> /dev/null
        chmod 0664 "${tz}"/trip_point_*_hyst 2> /dev/null
    fi
done
