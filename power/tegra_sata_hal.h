/*
 * Copyright (c) 2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef TEGRA_SATA_HAL_H
#define TEGRA_SATA_HAL_H

int hdd_get_temperature(const char* device_node, int *temperature);
int hdd_set_standby_timer(int timer_sec);
int hdd_disable_standby_timer();

#endif

