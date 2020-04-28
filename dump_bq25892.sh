#!/vendor/bin/sh

LOG_FILE="/data/misc/fw_logs/bq25892.txt"

/vendor/bin/fwtool ec usbpd 0 > ${LOG_FILE}
/vendor/bin/fwtool ec usbpdpower >> ${LOG_FILE}
/vendor/bin/fwtool ec bq25892 >> ${LOG_FILE}
/vendor/bin/fwtool ec pi3usb9281 >> ${LOG_FILE}
/vendor/bin/fwtool ec console >> ${LOG_FILE}
/vendor/bin/fwtool ec bq27742 >> ${LOG_FILE}
chmod a+r ${LOG_FILE}
