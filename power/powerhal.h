/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2012-2017, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef COMMON_POWER_HAL_H
#define COMMON_POWER_HAL_H

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "powerhal_utils.h"
#include "timeoutpoker.h"
#include <semaphore.h>

#include <vector>

#include <vendor/nvidia/hardware/power/1.0/IPower.h>

using ::vendor::nvidia::hardware::power::V1_0::ExtPowerHint;
using ::vendor::nvidia::hardware::power::V1_0::NvCPLHintData;

#define MAX_CHARS 32

#define POWER_CAP_PROP "persist.sys.NV_PBC_PWR_LIMIT"

//PMQOS control entry
#define PMQOS_CONSTRAINT_CPU_FREQ       "/dev/constraint_cpu_freq"
#define PMQOS_CONSTRAINT_GPU_FREQ       "/dev/constraint_gpu_freq"
#define PMQOS_CONSTRAINT_ONLINE_CPUS    "/dev/constraint_online_cpus"

//Default value to align with kernel pm qos
#define PM_QOS_DEFAULT_VALUE		-1

#define PRISM_CONTROL_PROP              "persist.sys.didim.enable"

#define PM_QOS_BOOST_PRIORITY 35
#define PM_QOS_APP_PROFILE_PRIORITY  40

#define HARDWARE_TYPE_PROP "ro.hardware"

#define POWER_HINT_MAX ExtPowerHint::FRAMERATE_DATA

struct input_dev_map {
    int dev_id;
    const char* dev_name;
};

typedef struct interactive_data {
    const char *hispeed_freq;
    const char *target_loads;
    const char *above_hispeed_delay;
    const char *timer_rate;
    const char *boost_factor;
    const char *min_sample_time;
    const char *go_hispeed_load;
} interactive_data_t;

typedef struct power_hint_data {
    int min;
    int max;
    int time_ms;
} power_hint_data_t;

typedef struct cpu_cluster_data {
    const char *pmqos_constraint_path;
    const char *available_freqs_path;
    int *available_frequencies;
    int num_available_frequencies;
    int fd_app_min_freq;
    int fd_app_max_freq;
    int fd_vsync_min_freq;

    std::map<ExtPowerHint,power_hint_data_t> hints;
} cpu_cluster_data_t;

struct powerhal_info {
    TimeoutPoker* mTimeoutPoker;

    std::vector<cpu_cluster_data_t> cpu_clusters;

    bool ftrace_enable;
    bool no_cpufreq_interactive;
    bool no_sclk_boost;

    /* Holds input devices */
    std::vector<struct input_dev_map> input_devs;

    /* Time last hint was sent - in usec */
    std::map<ExtPowerHint,uint64_t> hint_time;
    std::map<ExtPowerHint,uint64_t> hint_interval;

    std::map<ExtPowerHint,power_hint_data_t> gpu_freq_hints;
    std::map<ExtPowerHint,power_hint_data_t> emc_freq_hints;
    std::map<ExtPowerHint,power_hint_data_t> online_cpu_hints;

    int boot_boost_time_ms;

    /* AppProfile defaults */
    struct {
        int min_freq;
        int max_freq;
        int core_cap;
        int gpu_cap;
        int fan_cap;
        int power_cap;
    } defaults;

    /* Features on platform */
    struct {
        bool fan;
    } features;

    /* File descriptors used for hints and app profiles */
    struct {
        int app_max_online_cpus;
        int app_min_online_cpus;
        int app_max_gpu;
        int app_min_gpu;
    } fds;

    /* Switching CPU/EMC freq ratio based on display state */
    bool switch_cpu_emc_limit_enabled;
};

/* Opens power hw module */
void common_power_open(struct powerhal_info *pInfo);

/* Power management setup action at startup.
 * Such as to set default cpufreq parameters.
 */
void common_power_init(struct powerhal_info *pInfo);

/* Power management action,
 * upon the system entering interactive state and ready for interaction,
 * often with UI devices
 * OR
 * non-interactive state the system appears asleep, displayi/touch usually turned off.
*/
void common_power_set_interactive(struct powerhal_info *pInfo, int on);

/* PowerHint called to pass hints on power requirements, which
 * may result in adjustment of power/performance parameters of the
 * cpufreq governor and other controls.
*/
void common_power_hint(struct powerhal_info *pInfo, ExtPowerHint hint, const void *data);

void set_power_level_floor(int on);
#endif  //COMMON_POWER_HAL_H
