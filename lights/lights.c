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

#define LOG_TAG "lights"

#include <cutils/log.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>
#include <hardware/hardware.h>

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
char const* const BACKLIGHT_FILE = "/sys/class/backlight/lp8557-backlight/brightness";
char const* const PROGRAM_FILE = "/sys/class/chromeos/cros_ec/lightbar/program";
char const* const SEQUENCE_FILE = "/sys/class/chromeos/cros_ec/lightbar/sequence";

char lightbar_program[] = {
	0x0a,0xf0,0x00,0x00,0x00,	/* set.rgb {0,1,2,3}.beg 0x00 0x00 0x00 */
	0x0a,0xf4,0x00,0x00,0x00,	/* set.rgb {0,1,2,3}.end [color] */
	0x06,0x00,0x00,0x00,0x00,	/* delay.r 0 */
	0x05,0x00,0x03,0xd0,0x90,	/* L0001: delay.w [on_us] */
	0x00,				/* on */
	0x0d,				/* ramp.1 */
	0x07,				/* wait */
	0x0e,				/* cycle.1 */
	0x01,				/* off */
	0x05,0x00,0x0b,0x71,0xb0,	/* delay.w [off_us] */
	0x07,				/* wait */
	0x02,0x0f			/* jump L0001 */
};


static void program_pack_color(int red, int green, int blue)
{
	char *lightbar_color = &lightbar_program[7];
	lightbar_color[0] = red;
	lightbar_color[1] = green;
	lightbar_color[2] = blue;
}

static void program_pack_time_ms(char *which, int ms)
{
	int us = ms * 1000;
	which[0] = (us >> 24) & 0xff;
	which[1] = (us >> 16) & 0xff;
	which[2] = (us >>  8) & 0xff;
	which[3] =  us        & 0xff;
}

static void program_pack_on_off(int onMS, int offMS)
{
	char *lightbar_on_us = &lightbar_program[16];
	char *lightbar_off_us = &lightbar_program[26];
	program_pack_time_ms(lightbar_on_us, onMS);
	program_pack_time_ms(lightbar_off_us, offMS);
}

static int write_int(char const *path, int value)
{
	int fd;
	static int already_warned = -1;
	fd = open(path, O_RDWR);
	if (fd >= 0) {
		char buffer[20];
		int bytes = snprintf(buffer, 20, "%d\n", value);
		int amt = write(fd, buffer, bytes);
		close(fd);
		return amt == -1 ? -errno : 0;
	} else {
		if (already_warned == -1) {
			ALOGE("write_int failed to open %s\n", path);
			already_warned = 1;
		}
		return -errno;
	}
}

static int write_string(char const *path, char const *str, int len)
{
	int fd;
	static int already_warned = -1;
	fd = open(path, O_RDWR);
	if (fd >= 0) {
		int amt = write(fd, str, len);
		close(fd);
		return amt == -1 ? -errno : 0;
	} else {
		if (already_warned == -1) {
			ALOGE("write_int failed to open %s\n", path);
			already_warned = 1;
		}
		return -errno;
	}
}

static int rgb_to_brightness(struct light_state_t const *state)
{
	int color = state->color & 0x00ffffff;
	return ((77 * ((color >> 16) & 0x00ff))
			+ (150 * ((color >> 8) & 0x00ff)) +
			(29 * (color & 0x00ff))) >> 8;
}

static int set_light_backlight(struct light_device_t *dev,
		struct light_state_t const *state)
{
	int err;
	int brightness = rgb_to_brightness(state);

	pthread_mutex_lock(&g_lock);
	err = write_int(BACKLIGHT_FILE, brightness);
	pthread_mutex_unlock(&g_lock);

	return err;
}

static int set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
	int len;
	int red, green, blue;
	int onMS, offMS;
	unsigned int colorRGB;

	pthread_mutex_lock(&g_lock);

	switch (state->flashMode) {
	case LIGHT_FLASH_TIMED:
	case LIGHT_FLASH_HARDWARE:
		onMS = state->flashOnMS;
		offMS = state->flashOffMS;
		break;
	case LIGHT_FLASH_NONE:
	default:
		onMS = 0;
		offMS = 0;
		break;
	}

	colorRGB = state->color;

#if DEBUG
	ALOGD("set_light_locked mode %d, colorRGB=%08X, onMS=%d, offMS=%d\n",
		state->flashMode, colorRGB, onMS, offMS);
#endif

	red = (colorRGB >> 16) & 0xff;
	green = (colorRGB >> 8) & 0xff;
	blue = colorRGB & 0xff;

	if (onMS == 0) {
		pthread_mutex_unlock(&g_lock);
		return 0;
	}

	program_pack_color(red, green, blue);
	program_pack_on_off(onMS, offMS);
	write_string(PROGRAM_FILE,
		lightbar_program, sizeof(lightbar_program));
	write_string(SEQUENCE_FILE, "program", 7);

	pthread_mutex_unlock(&g_lock);
	return 0;
}

/** Close the lights device */
static int close_lights(struct light_device_t *dev)
{
	if (dev)
		free(dev);
	return 0;
}

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t *module, char const *name,
		struct hw_device_t **device)
{
	struct light_device_t *dev = malloc(sizeof(struct light_device_t));
	int (*set_light) (struct light_device_t *dev,
			struct light_state_t const *state);
	pthread_t lighting_poll_thread;
	ALOGV("open lights");
	if (dev == NULL) {
		ALOGE("failed to allocate memory");
		return -1;
	}
	memset(dev, 0, sizeof(*dev));

	if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
		set_light = set_light_backlight;
	else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name) ||
		 0 == strcmp(LIGHT_ID_ATTENTION, name))
		set_light = set_light_notifications;
	else
		return -EINVAL;

	pthread_mutex_init(&g_lock, NULL);

	dev->common.tag = HARDWARE_DEVICE_TAG;
	dev->common.version = 0;
	dev->common.module = (struct hw_module_t *)module;
	dev->common.close = (int (*)(struct hw_device_t *))close_lights;
	dev->set_light = set_light;

	*device = (struct hw_device_t *)dev;

	return 0;
}

static struct hw_module_methods_t lights_methods =
{
	.open =  open_lights,
};

/*
 * The backlight Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM =
{
	.tag = HARDWARE_MODULE_TAG,
	.version_major = 1,
	.version_minor = 0,
	.id = LIGHTS_HARDWARE_MODULE_ID,
	.name = "dragon lights module",
	.author = "Google, Inc.",
	.methods = &lights_methods,
};
