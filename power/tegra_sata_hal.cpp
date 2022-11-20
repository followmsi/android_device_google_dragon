/*
 * Copyright (c) 2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <linux/hdreg.h>
#include <errno.h>
#include <cutils/log.h>
#include "tegra_sata_hal.h"

#define STATUS_SIZE 512
#define STATUS_OFFSET 4
#define HDA_TEMP_OFFSETOFFSET 200
#define HDA_TEMP (STATUS_OFFSET + HDA_TEMP_OFFSETOFFSET)

/* Need better method instead of Hard code the sysfs node */
#define POWER_CONTROL_PATH "/sys/devices/platform/tegra-sata.0/ata1/power/control"
#define AUTO_SUSPEND_DELAY_PATH "/sys/devices/platform/tegra-sata.0/ata1/host0/target0:0:0/0:0:0:0/power/autosuspend_delay_ms"
#define HOST_CONTROL_PATH "/sys/devices/platform/tegra-sata.0/ata1/host0/target0:0:0/0:0:0:0/power/control"
#define VALUE_AUTO "auto"
#define VALUE_ON "on"

#define IF_ERROR_EXIT( return_code, output_err_msg ) do { \
            error = return_code; \
                if (error < 0) { \
                    output_err_msg; \
                    goto done; \
                    } \
         } while(0);

/*
 ******************************************************************************
 Common logic for all commands
 *****************************************************************************
 */
static int check_node_existence(const char* node)
{
    int error;

    IF_ERROR_EXIT(access(node, F_OK), ALOGE("HAL: sysfs %s not exist", node));

    IF_ERROR_EXIT(access(node, R_OK|W_OK), ALOGE("HAL: sysfs %s permission is not proper", node));

    return 0;

done:
    return error;

}

static int tegra_sata_hal_ioctl(int fd, int command, unsigned char *buf)
{
    return ioctl(fd, command, buf);
}


/*
 ******************************************************************************
 Temperature related logics
 *****************************************************************************
 */

/*
 * Given a device_node, return the temperature of the device
 * If the query is successful, return 0. If not, return a non zero number
 * The temperature should be returned by setting the *temperature
 */
int hdd_get_temperature(const char* device_node, int *temperature)
{
    unsigned char args[STATUS_OFFSET + STATUS_SIZE] = {0};
    int fd = -1;
    int error = 0;

    IF_ERROR_EXIT(check_node_existence(device_node), ALOGE("HAL: Node %s doesnt exit", device_node));

    args[0] = WIN_SMART;             /* COMMAND */
    args[1] = 0xE0;                  /* NSECTOR */
    args[2] = SMART_READ_LOG_SECTOR; /* FEATURE */
    args[3] = 0x1;                   /* NSECTOR */

    IF_ERROR_EXIT(open(device_node, O_RDONLY | O_NONBLOCK), ALOGE("HAL: Failed to open %s", device_node));
    fd = error;

    IF_ERROR_EXIT(tegra_sata_hal_ioctl(fd, HDIO_DRIVE_CMD, args), ALOGE("HAL: Ioctl failed in %s", device_node));
    close(fd);

    /* Need to check if there is any error in status */
    *temperature = (int)args[HDA_TEMP];
    ALOGD("HAL: HDA_TEMP = %d", *temperature);
    return 0;

done:
    if (fd >= 0)
        close(fd);
    return error;
}

/*
 ******************************************************************************
 Standby and its related logics
 *****************************************************************************
 */

int hdd_set_standby_timer(int timer_sec)
{
    int error = 0;
    int timer_ms = 0;
    int fd = -1;
    char buf[80];

    if (timer_sec < 0) {
        ALOGE("HAL: Invalid input time for Standby");
        return -EINVAL;
    }
    timer_ms = timer_sec * 1000;
    sprintf(buf, "%d", timer_ms);

    /* Implement this function base on RTPM */
    /* Setp1: Set power control to be "auto"*/
    IF_ERROR_EXIT(open(POWER_CONTROL_PATH, O_WRONLY), ALOGE("HAL: Failed to open %s", POWER_CONTROL_PATH));
    fd = error;

    IF_ERROR_EXIT(write(fd, VALUE_AUTO, strlen(VALUE_AUTO)), ALOGE("HAL: Failed to set %s , fd=%d, returned '%s'(%d)", POWER_CONTROL_PATH, fd, strerror(errno), errno));
    close(fd);
    fd = -1;

    /* Step2: Set suspend delay time */
    IF_ERROR_EXIT(open(AUTO_SUSPEND_DELAY_PATH, O_WRONLY), ALOGE("HAL: Failed to open %s", AUTO_SUSPEND_DELAY_PATH));
    fd = error;

    IF_ERROR_EXIT(write(fd, buf, strlen(buf)), ALOGE("HAL: Failed to set %s , fd=%d, returned '%s'(%d)", AUTO_SUSPEND_DELAY_PATH, fd, strerror(errno), errno));
    close(fd);
    fd = -1;

    /* Step3: Set host power control to be "auto"*/
    IF_ERROR_EXIT(open(HOST_CONTROL_PATH, O_WRONLY), ALOGE("HAL: Failed to open %s", HOST_CONTROL_PATH));
    fd = error;

    IF_ERROR_EXIT(write(fd, VALUE_AUTO, strlen(VALUE_AUTO)), ALOGE("HAL: Failed to set %s , fd=%d, returned '%s'(%d)", HOST_CONTROL_PATH, fd, strerror(errno), errno));
    close(fd);

    ALOGI("HAL: Trying to enter Standby mode(HDD spin-down) in %d sec...", timer_sec);
    return 0;

done:
    if (fd >= 0)
        close(fd);
    return error;
}

int hdd_disable_standby_timer()
{
    int error = 0;
    int fd = -1;

    /* Implement this function base on RTPM */
    /* Setp1: Set power control to be "on" */
    fd = open(POWER_CONTROL_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("HAL: Failed to open %s", POWER_CONTROL_PATH);
        return -1;
    }

    error = write(fd, VALUE_ON, strlen(VALUE_ON));
    if (error < 0) {
        ALOGE("HAL: Failed to set %s , fd=%d, returned '%s'(%d)", POWER_CONTROL_PATH, fd, strerror(errno), errno);
        goto done;
    }
    close(fd);

    /* Step2: Set host power control to be "on" */
    fd = open(HOST_CONTROL_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("HAL: Failed to open %s", HOST_CONTROL_PATH);
        return -1;
    }

    error = write(fd, VALUE_ON, strlen(VALUE_ON));
    if (error < 0) {
        ALOGE("HAL: Failed to set %s , fd=%d, returned '%s'(%d)", HOST_CONTROL_PATH, fd, strerror(errno), errno);
        goto done;
    }
    close(fd);

    ALOGI("HAL: Standby mode(HDD spin-down timeout) has been disabled.");
    return 0;

done:
    close(fd);
    return error;
}

