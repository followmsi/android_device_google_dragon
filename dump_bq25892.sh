#!/system/bin/sh

LOG_FILE="/data/misc/fw_logs/bq25892.txt"

/system/bin/fwtool ec usbpd 0 > ${LOG_FILE}
/system/bin/fwtool ec usbpdpower >> ${LOG_FILE}
/system/bin/fwtool ec bq25892 >> ${LOG_FILE}
chmod a+r ${LOG_FILE}
