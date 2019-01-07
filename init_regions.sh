#!/system/bin/sh
#
# Intialize region settings. Ref: crosbug.com/p/44779

REGION_VPD_FILE=/sys/firmware/vpd/ro/region

REGION="us"
LANGUAGE="en"
COUNTRY="US"

if [ -f "${REGION_VPD_FILE}" ]; then
  REGION="$(cat ${REGION_VPD_FILE})"
fi

case "${REGION}" in
  gb | ie)
    COUNTRY="GB"
    ;;
  au | nz)
    COUNTRY="AU"
    ;;
  de)
    COUNTRY="DE"
    ;;
esac

setprop ro.product.locale "${LANGUAGE}-${COUNTRY}"

OVERRIDE_CC=$(getprop persist.sys.wifi.country_code)
CURRENT_BOOT_CC=$(getprop ro.boot.wificountrycode)

if [ -n "$OVERRIDE_CC" ]; then
  COUNTRY=$OVERRIDE_CC
fi

if [ -z "$CURRENT_BOOT_CC" ]; then
  setprop ro.boot.wificountrycode "${COUNTRY}"
elif [ "$CURRENT_BOOT_CC" != "${COUNTRY}" -a -x /sbin/resetprop ]; then
  # Preferred country code changed during boot? Likely Magisk
  # changed the boot procedure and we have already been called once
  # with props missing. Now use Magisk's tool to settle the problem
  # it causes.
  /sbin/resetprop ro.boot.wificountrycode "${COUNTRY}"
fi
