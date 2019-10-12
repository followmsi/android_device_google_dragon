/*
 * Copyright (C) 2016-2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.health@2.0-service.dragon"

#include <cutils/klog.h>
#include <fcntl.h>
#include <healthd/BatteryMonitor.h>
#include <healthd/healthd.h>
#include <health2/service.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#define PSU_SYSFS_PATH "/sys/class/power_supply/bq27742-0"
#define PSU_SYSFS_MAX_CURRENT_PATH PSU_SYSFS_PATH "/current_max"
#define BATTERY_CRITICAL_LOW_CAP 10
#define BATTERY_CRITICAL_LOW_IMAX_MA 5000
#define BATTERY_MAX_IMAX_MA          9000

int read_sysfs(const char *path, char *buf, size_t size) {
    char *cp = NULL;

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        KLOG_ERROR(LOG_TAG, "Could not open '%s'\n", path);
        return -1;
    }

    ssize_t count = TEMP_FAILURE_RETRY(read(fd, buf, size));
    if (count > 0)
        cp = (char *)memrchr(buf, '\n', count);

    if (cp)
        *cp = '\0';
    else
        buf[0] = '\0';

    //close(fd);
    return count;
}

int read_current_max_ma() {
    const int SIZE = 10;
    char buf[SIZE];
    if (read_sysfs(PSU_SYSFS_MAX_CURRENT_PATH, buf, SIZE) > 0)
        return atoi(buf) / 1000;

    return 0;
}

void dragon_soc_adjust(struct android::BatteryProperties* props)
{
    int soc;
    int current_max_ma;

    soc = props->batteryLevel;
    /*
     * if not charging and State-Of-Charge (soc) is below
     * BATTERY_CRITICAL_LOW_CAP shrink soc based on imax value
     */
    if ((soc < BATTERY_CRITICAL_LOW_CAP) &&
        ((props->batteryStatus == android::BATTERY_STATUS_DISCHARGING) ||
         (props->batteryStatus == android::BATTERY_STATUS_NOT_CHARGING) ||
         (props->batteryStatus == android::BATTERY_STATUS_UNKNOWN))) {

        current_max_ma = read_current_max_ma();
        if (current_max_ma == 0)
            /*
             * Either it failed to read sysfs or its really zero.  In either
             * case lets just warn so logs will identify for further debug.
             */
            KLOG_WARNING(LOG_TAG, "imax=0\n");
        else if (current_max_ma < BATTERY_CRITICAL_LOW_IMAX_MA)
            /* force shutdown */
            soc = 0;
        else if (current_max_ma < BATTERY_MAX_IMAX_MA)
            /* decrease soc towards zero */
            soc = soc * current_max_ma / BATTERY_MAX_IMAX_MA;

        KLOG_INFO(LOG_TAG, "imax=%d soc=%d\n", current_max_ma, soc);
    }
    props->batteryLevel = soc;
}

int healthd_board_battery_update(struct android::BatteryProperties* props) {

    dragon_soc_adjust(props);

    // return 0 to log periodic polled battery status to kernel log
    return 0;
}

void healthd_board_init(struct healthd_config*) {}

int main() {
    return health_service_main();
}
