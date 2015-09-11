/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cutils/properties.h>
//#define LOG_NDEBUG 0

#define LOG_TAG "DragonPowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "timed_qos_manager.h"

#define BOOSTPULSE_PATH "/sys/devices/system/cpu/cpufreq/interactive/boostpulse"
#define CPU_MAX_FREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPU_QUIET_NR_MIN_CPUS_PATH "/sys/devices/system/cpu/cpuquiet/nr_min_cpus"
#define TOUCH_SYNA_INTERACTIVE_PATH "/sys/bus/i2c/devices/0-0020/0018:06CB:3370.0001/input/input0/inhibited"
#define IO_IS_BUSY_PATH "/sys/devices/system/cpu/cpufreq/interactive/io_is_busy"
#define LIGHTBAR_SEQUENCE_PATH "/sys/class/chromeos/cros_ec/lightbar/sequence"
#define LOW_POWER_MAX_FREQ "1020000"
#define NORMAL_MAX_FREQ "1912500"
#define GPU_BOOST_PATH "/sys/devices/57000000.gpu/pstate"
#define GPU_BOOST_ENTER_CMD "03,0C"    // boost GPU to work at least on 03 - 230MHz
#define GPU_BOOST_EXIT_CMD "auto"
#define GPU_FREQ_CONSTRAINT "852000 852000 -1 2000"

struct dragon_power_module {
    struct power_module base;
    pthread_mutex_t boost_pulse_lock;
    pthread_mutex_t low_power_lock;
    int boostpulse_fd;
    int boostpulse_warned;
    TimedQosManager *gpu_qos_manager;
};

static bool low_power_mode = false;

static const char *max_cpu_freq = NORMAL_MAX_FREQ;
static const char *low_power_max_cpu_freq = LOW_POWER_MAX_FREQ;


void sysfs_write(const char *path, const char *s)
{
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

static void power_init(struct power_module __unused *module)
{
    struct dragon_power_module *dragon =
            (struct dragon_power_module *) module;

    dragon->gpu_qos_manager = new TimedQosManager("GPU",
        new SysfsQosObject(GPU_BOOST_PATH, GPU_BOOST_ENTER_CMD, GPU_BOOST_EXIT_CMD),
        false);
    dragon->gpu_qos_manager->run("GpuTimedQosManager", PRIORITY_FOREGROUND);

    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_rate",
                "20000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_slack",
                "20000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/min_sample_time",
                "80000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq",
                "1530000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load",
                "99");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/target_loads",
                "65 228000:75 624000:85");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay",
                "20000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/boostpulse_duration",
                "1000000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/io_is_busy", "0");
    sysfs_write("/sys/class/chromeos/cros_ec/lightbar/userspace_control", "1");
}

static void power_set_interactive(struct power_module __unused *module, int on)
{
    ALOGV("power_set_interactive: %d\n", on);

    /*
     * Lower maximum frequency when screen is off.
     */
    sysfs_write(CPU_MAX_FREQ_PATH,
                (!on || low_power_mode) ? low_power_max_cpu_freq : max_cpu_freq);
    sysfs_write(IO_IS_BUSY_PATH, on ? "1" : "0");
    sysfs_write(TOUCH_SYNA_INTERACTIVE_PATH, on ? "0" : "1");
    sysfs_write(LIGHTBAR_SEQUENCE_PATH, on ? "s3s0" : "s0s3");
    sysfs_write(CPU_QUIET_NR_MIN_CPUS_PATH, on ? "2" : "1");
    ALOGV("power_set_interactive: %d done\n", on);
}

static int boostpulse_open(struct dragon_power_module *dragon)
{
    char buf[80];
    int len;

    pthread_mutex_lock(&dragon->boost_pulse_lock);

    if (dragon->boostpulse_fd < 0) {
        dragon->boostpulse_fd = open(BOOSTPULSE_PATH, O_WRONLY);

        if (dragon->boostpulse_fd < 0) {
            if (!dragon->boostpulse_warned) {
                strerror_r(errno, buf, sizeof(buf));
                ALOGE("Error opening %s: %s\n", BOOSTPULSE_PATH, buf);
                dragon->boostpulse_warned = 1;
            }
        }
    }

    pthread_mutex_unlock(&dragon->boost_pulse_lock);
    return dragon->boostpulse_fd;
}

static void dragon_power_hint(struct power_module *module, power_hint_t hint,
                                void *data)
{
    struct dragon_power_module *dragon =
            (struct dragon_power_module *) module;
    char buf[80];
    int len;

    switch (hint) {
    case POWER_HINT_INTERACTION:
        if (boostpulse_open(dragon) >= 0) {
            len = write(dragon->boostpulse_fd, "1", 1);

            if (len < 0) {
                strerror_r(errno, buf, sizeof(buf));
                ALOGE("Error writing to %s: %s\n", BOOSTPULSE_PATH, buf);
            }
        }
        if (dragon->gpu_qos_manager != NULL)
            dragon->gpu_qos_manager->requestTimedQos(s2ns(1));

        break;

    case POWER_HINT_VSYNC:
        break;

    case POWER_HINT_LOW_POWER:
        pthread_mutex_lock(&dragon->low_power_lock);
        if (data) {
            sysfs_write(CPU_MAX_FREQ_PATH, low_power_max_cpu_freq);
        } else {
            sysfs_write(CPU_MAX_FREQ_PATH, max_cpu_freq);
        }
        low_power_mode = data;
        pthread_mutex_unlock(&dragon->low_power_lock);
        break;

    default:
            break;
    }
}

static int dragon_power_open(const hw_module_t *module, const char *name,
                            hw_device_t **device)
{
    return 0;
}


static struct hw_module_methods_t power_module_methods = {
    .open = dragon_power_open,
};

struct dragon_power_module HAL_MODULE_INFO_SYM = {
    base: {
        common: {
            tag: HARDWARE_MODULE_TAG,
            module_api_version: POWER_MODULE_API_VERSION_0_2,
            hal_api_version: HARDWARE_HAL_API_VERSION,
            id: POWER_HARDWARE_MODULE_ID,
            name: "Dragon Power HAL",
            author: "The Android Open Source Project",
            methods: &power_module_methods,
            dso: NULL,
            reserved: {0},
        },

        init: power_init,
        setInteractive: power_set_interactive,
        powerHint: dragon_power_hint,
    },

    boost_pulse_lock: PTHREAD_MUTEX_INITIALIZER,
    low_power_lock: PTHREAD_MUTEX_INITIALIZER,
    boostpulse_fd: -1,
    boostpulse_warned: 0,
    gpu_qos_manager: NULL,
};

