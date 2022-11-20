/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2013-2016, NVIDIA CORPORATION.  All rights reserved.
 * Copyright (C) 2019 The LineageOS Project
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

#ifndef POWER_HAL_UTILS_H
#define POWER_HAL_UTILS_H

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <utils/Log.h>
#include <cutils/properties.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

/* sysfs utilities */
void sysfs_write(const char *path, const char *s);
void sysfs_write_int(const char *path, int value);
void sysfs_read(const char *path, char *s, int size);
bool sysfs_exists(const char *path);

/* Property utilities */
bool get_property_bool(const char *key, bool default_value);
void set_property_int(const char *key, int value);
void set_property_int(const char *key, int value);

/* Governor utilities */
int get_scaling_governor(char governor[], int size);
int get_scaling_governor_check_cores(char governor[], int size, int core_num);
int is_interactive_governor(char*);
int is_schedutil_governor(char*);

#endif  // POWER_HAL_UTILS_H

