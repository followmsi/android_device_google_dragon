/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2013-2017, NVIDIA CORPORATION.  All rights reserved.
 * Copyright (C) 2019 The LineageOS Project
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
#define LOG_TAG "powerHAL::common"

#include "powerhal_utils.h"
#include <stdio.h>
#include <unistd.h>

#define INTERACTIVE_GOVERNOR "interactive"
#define SCHEDUTIL_GOVERNOR "schedutil"

const char* scaling_gov_path[8] = {"/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu2/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu3/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu4/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu5/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu6/cpufreq/scaling_governor",
                                   "/sys/devices/system/cpu/cpu7/cpufreq/scaling_governor"};

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

void sysfs_read(const char *path, char *s, int size)
{
    int len;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        strerror_r(errno, s, size);
        ALOGE("Error opening %s: %s\n", path, s);
        return;
    }

    len = read(fd, s, size);
    close(fd);

    if (len == size)
        s[len - 1] = '\0';

    if (len < 0) {
        strerror_r(errno, s, size);
        ALOGE("Error reading from %s: %s\n", path, s);
    }
}

bool sysfs_exists(const char *path)
{
    bool val;
    int fd = open(path, O_RDONLY);

    val = fd < 0 ? false : true;
    close(fd);

    return val;
}

bool get_property_bool(const char *key, bool default_value)
{
    char value[PROPERTY_VALUE_MAX];

    if (property_get(key, value, NULL) > 0) {
        if (!strcmp(value, "1") || !strcasecmp(value, "on") ||
            !strcasecmp(value, "true")) {
            return true;
        }
        if (!strcmp(value, "0") || !strcasecmp(value, "off") ||
            !strcasecmp(value, "false")) {
            return false;
        }
    }

    return default_value;
}

void set_property_int(const char *key, int value)
{
    char val[PROPERTY_VALUE_MAX];
    int status;

    snprintf(val, sizeof(val), "%d", value);
    status = property_set(key, val);

    if (status) {
        ALOGE("Error writing to property: %s\n", key);
    }
}

void sysfs_write_int(const char *path, int value)
{
    char val[PROPERTY_VALUE_MAX];

    snprintf(val, sizeof(val), "%d", value);
    sysfs_write(path, val);
}

int get_scaling_governor(char governor[], int size) {
    for (size_t i = 0; i < ARRAY_SIZE(scaling_gov_path); i++) {
        if (get_scaling_governor_check_cores(governor, size, i) == 0) {
            // Obtained the scaling governor. Return.
            return 0;
        }
    }

    return -1;
}

int get_scaling_governor_check_cores(char governor[], int size, int core_num) {
    sysfs_read(scaling_gov_path[core_num], governor, size);

    if (governor[0] == '\0')
        return -1;

    // Strip newline at the end.
    int len = strlen(governor);
    len--;
    while (len >= 0 && (governor[len] == '\n' || governor[len] == '\r')) governor[len--] = '\0';

    return 0;
}

int is_interactive_governor(char* governor) {
    if (strncmp(governor, INTERACTIVE_GOVERNOR, (strlen(INTERACTIVE_GOVERNOR) + 1)) == 0) return 1;
    return 0;
}

int is_schedutil_governor(char* governor) {
    if (strncmp(governor, SCHEDUTIL_GOVERNOR, (strlen(SCHEDUTIL_GOVERNOR) + 1)) == 0) return 1;
    return 0;
}
