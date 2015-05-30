/*
 * Copyright (C) 2015 The Android Open Source Project
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
/* Useful direct commands to the EC host interface */

#define LOG_TAG "fwtool"

#include <cutils/log.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ec_commands.h"
#include "debug_cmd.h"
#include "flash_device.h"

static void *ec;

static void *get_ec(void)
{
	if (!ec)
		ec = flash_open("ec", NULL);

	return ec;
}

static int ec_readmem(int offset, int bytes, void *dest)
{
	struct ec_params_read_memmap r_mem;
	int r;

	r_mem.offset = offset;
	r_mem.size = bytes;
	return flash_cmd(ec, EC_CMD_READ_MEMMAP, 0,
			 &r_mem, sizeof(r_mem), dest, bytes);
}

static uint8_t ec_readmem8(int offset)
{
	uint8_t val;
	int ret;
	ret = ec_readmem(offset, sizeof(val), &val);
	return ret ? 0 : val;
}

static uint32_t ec_readmem32(int offset)
{
	uint32_t val;
	int ret;
	ret = ec_readmem(offset, sizeof(val), &val);
	return ret ? 0 : val;
}

static int cmd_ec_battery(int argc, const char **argv)
{
	char batt_text[EC_MEMMAP_TEXT_MAX + 1];
	uint32_t val;

	if (!get_ec())
		return -ENODEV;

	printf("Battery info:\n");

	val = ec_readmem8(EC_MEMMAP_BATTERY_VERSION);
	if (val < 1) {
		fprintf(stderr, "Battery version %d is not supported\n", val);
		return -EINVAL;
	}

	memset(batt_text, 0, EC_MEMMAP_TEXT_MAX + 1);
	ec_readmem(EC_MEMMAP_BATT_MFGR, sizeof(batt_text), batt_text);
	printf("  OEM name:               %s\n", batt_text);
	ec_readmem(EC_MEMMAP_BATT_MODEL, sizeof(batt_text), batt_text);
	printf("  Model number:           %s\n", batt_text);
	printf("  Chemistry   :           %s\n", batt_text);
	ec_readmem(EC_MEMMAP_BATT_SERIAL, sizeof(batt_text), batt_text);
	printf("  Serial number:          %s\n", batt_text);
	val = ec_readmem32(EC_MEMMAP_BATT_DCAP);
	printf("  Design capacity:        %u mAh\n", val);
	val = ec_readmem32(EC_MEMMAP_BATT_LFCC);
	printf("  Last full charge:       %u mAh\n", val);
	val = ec_readmem32(EC_MEMMAP_BATT_DVLT);
	printf("  Design output voltage   %u mV\n", val);
	val = ec_readmem32(EC_MEMMAP_BATT_CCNT);
	printf("  Cycle count             %u\n", val);
	val = ec_readmem32(EC_MEMMAP_BATT_VOLT);
	printf("  Present voltage         %u mV\n", val);
	val = ec_readmem32(EC_MEMMAP_BATT_RATE);
	printf("  Present current         %u mA\n", val);
	val = ec_readmem32(EC_MEMMAP_BATT_CAP);
	printf("  Remaining capacity      %u mAh\n", val);
	val = ec_readmem8(EC_MEMMAP_BATT_FLAG);
	printf("  Flags                   0x%02x", val);
	if (val & EC_BATT_FLAG_AC_PRESENT)
		printf(" AC_PRESENT");
	if (val & EC_BATT_FLAG_BATT_PRESENT)
		printf(" BATT_PRESENT");
	if (val & EC_BATT_FLAG_DISCHARGING)
		printf(" DISCHARGING");
	if (val & EC_BATT_FLAG_CHARGING)
		printf(" CHARGING");
	if (val & EC_BATT_FLAG_LEVEL_CRITICAL)
		printf(" LEVEL_CRITICAL");
	printf("\n");

	return 0;
}

static void print_pd_power_info(struct ec_response_usb_pd_power_info *r)
{
	switch (r->role) {
	case USB_PD_PORT_POWER_DISCONNECTED:
		printf("Disconnected");
		break;
	case USB_PD_PORT_POWER_SOURCE:
		printf("SRC");
		break;
	case USB_PD_PORT_POWER_SINK:
		printf("SNK");
		break;
	case USB_PD_PORT_POWER_SINK_NOT_CHARGING:
		printf("SNK (not charging)");
		break;
	default:
		printf("Unknown");
	}

	if ((r->role == USB_PD_PORT_POWER_DISCONNECTED) ||
	    (r->role == USB_PD_PORT_POWER_SOURCE)) {
		printf("\n");
		return;
	}

	printf(r->dualrole ? " DRP" : " Charger");
	switch (r->type) {
	case USB_CHG_TYPE_PD:
		printf(" PD");
		break;
	case USB_CHG_TYPE_C:
		printf(" Type-C");
		break;
	case USB_CHG_TYPE_PROPRIETARY:
		printf(" Proprietary");
		break;
	case USB_CHG_TYPE_BC12_DCP:
		printf(" DCP");
		break;
	case USB_CHG_TYPE_BC12_CDP:
		printf(" CDP");
		break;
	case USB_CHG_TYPE_BC12_SDP:
		printf(" SDP");
		break;
	case USB_CHG_TYPE_OTHER:
		printf(" Other");
		break;
	case USB_CHG_TYPE_VBUS:
		printf(" VBUS");
		break;
	case USB_CHG_TYPE_UNKNOWN:
		printf(" Unknown");
		break;
	}
	printf(" %dmV / %dmA, max %dmV / %dmA",
		r->meas.voltage_now, r->meas.current_lim, r->meas.voltage_max,
		r->meas.current_max);
	if (r->max_power)
		printf(" / %dmW", r->max_power / 1000);
	printf("\n");
}

static int cmd_ec_usbpdpower(int argc, const char **argv)
{
	struct ec_params_usb_pd_power_info p;
	struct ec_response_usb_pd_power_info r;
	struct ec_response_usb_pd_ports rp;
	int i, rv;

	if (!get_ec())
		return -ENODEV;

	rv = flash_cmd(ec, EC_CMD_USB_PD_PORTS, 0, NULL, 0, &rp, sizeof(rp));
	if (rv)
		return rv;

	for (i = 0; i < rp.num_ports; i++) {
		p.port = i;
		rv = flash_cmd(ec, EC_CMD_USB_PD_POWER_INFO, 0,
				&p, sizeof(p), &r, sizeof(r));
		if (rv)
			return rv;

		printf("Port %d: ", i);
		print_pd_power_info(&r);
	}

	return 0;
}

struct command subcmds_ec[] = {
	CMD(ec_battery, "Show battery status"),
	CMD(ec_usbpdpower, "Information about USB PD ports"),
	CMD_GUARD_LAST
};
