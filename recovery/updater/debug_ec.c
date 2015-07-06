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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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

static int cmd_ec_gpioget(int argc, const char **argv)
{
	struct ec_params_gpio_get_v1 p_v1;
	struct ec_response_gpio_get_v1 r_v1;
	int i, rv, subcmd, num_gpios;

	if (!get_ec())
		return -ENODEV;

	if (argc > 2) {
		printf("Usage: %s [<subcmd> <GPIO name>]\n", argv[0]);
		printf("'gpioget <GPIO_NAME>' - Get value by name\n");
		printf("'gpioget count' - Get count of GPIOS\n");
		printf("'gpioget all' - Get info for all GPIOs\n");
		return -1;
	}

	/* Keeping it consistent with console command behavior */
	if (argc == 1)
		subcmd = EC_GPIO_GET_INFO;
	else if (!strcmp(argv[1], "count"))
		subcmd = EC_GPIO_GET_COUNT;
	else if (!strcmp(argv[1], "all"))
		subcmd = EC_GPIO_GET_INFO;
	else
		subcmd = EC_GPIO_GET_BY_NAME;

	if (subcmd == EC_GPIO_GET_BY_NAME) {
		p_v1.subcmd = EC_GPIO_GET_BY_NAME;
		if (strlen(argv[1]) + 1 > sizeof(p_v1.get_value_by_name.name)) {
			fprintf(stderr, "GPIO name too long.\n");
			return -1;
		}
		strcpy(p_v1.get_value_by_name.name, argv[1]);

		rv = flash_cmd(ec, EC_CMD_GPIO_GET, 1, &p_v1,
				sizeof(p_v1), &r_v1, sizeof(r_v1));

		if (rv < 0)
			return rv;

		printf("GPIO %s = %d\n", p_v1.get_value_by_name.name,
			r_v1.get_value_by_name.val);
		return 0;
	}

	/* Need GPIO count for EC_GPIO_GET_COUNT or EC_GPIO_GET_INFO */
	p_v1.subcmd = EC_GPIO_GET_COUNT;
	rv = flash_cmd(ec, EC_CMD_GPIO_GET, 1, &p_v1,
			sizeof(p_v1), &r_v1, sizeof(r_v1));
	if (rv < 0)
		return rv;

	if (subcmd == EC_GPIO_GET_COUNT) {
		printf("GPIO COUNT = %d\n", r_v1.get_count.val);
		return 0;
	}

	/* subcmd EC_GPIO_GET_INFO */
	num_gpios = r_v1.get_count.val;
	p_v1.subcmd = EC_GPIO_GET_INFO;

	for (i = 0; i < num_gpios; i++) {
		p_v1.get_info.index = i;

		rv = flash_cmd(ec, EC_CMD_GPIO_GET, 1, &p_v1,
				sizeof(p_v1), &r_v1, sizeof(r_v1));
		if (rv < 0)
			return rv;

		printf("%2d %-32s 0x%04X\n", r_v1.get_info.val,
			r_v1.get_info.name, r_v1.get_info.flags);
	}

	return 0;
}


static int cmd_ec_gpioset(int argc, const char **argv)
{
	struct ec_params_gpio_set p;
	char *e;
	int rv;

	if (!get_ec())
		return -ENODEV;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <GPIO name> <0 | 1>\n", argv[0]);
		return -1;
	}

	if (strlen(argv[1]) + 1 > sizeof(p.name)) {
		fprintf(stderr, "GPIO name too long.\n");
		return -1;
	}
	strcpy(p.name, argv[1]);

	p.val = strtol(argv[2], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Bad value.\n");
		return -1;
	}

	rv = flash_cmd(ec, EC_CMD_GPIO_SET, 0, &p, sizeof(p), NULL, 0);
	if (rv < 0)
		return rv;

	printf("GPIO %s set to %d\n", p.name, p.val);
	return 0;
}

#define LIGHTBAR_NUM_SEQUENCES 13

static int lb_do_cmd(enum lightbar_command cmd,
		     struct ec_params_lightbar *in,
		     struct ec_response_lightbar *out)
{
	int rv;
	in->cmd = cmd;
	rv = flash_cmd(ec, EC_CMD_LIGHTBAR_CMD, 0,
			in, 120,
			out, 120);
	return (rv < 0 ? rv : 0);
}

static int cmd_ec_lightbar(int argc, const char **argv)
{
	unsigned i;
	int r;
	struct ec_params_lightbar param;
	struct ec_response_lightbar resp;

	if (!get_ec())
		return -ENODEV;

	if (1 == argc) {		/* no args = dump 'em all */
		r = lb_do_cmd(LIGHTBAR_CMD_DUMP, &param, &resp);
		if (r)
			return r;
		for (i = 0; i < ARRAY_SIZE(resp.dump.vals); i++) {
			printf(" %02x     %02x     %02x\n",
			       resp.dump.vals[i].reg,
			       resp.dump.vals[i].ic0,
			       resp.dump.vals[i].ic1);
		}
		return 0;
	}

	if (argc == 2 && !strcasecmp(argv[1], "init"))
		return lb_do_cmd(LIGHTBAR_CMD_INIT, &param, &resp);

	if (argc == 2 && !strcasecmp(argv[1], "off"))
		return lb_do_cmd(LIGHTBAR_CMD_OFF, &param, &resp);

	if (argc == 2 && !strcasecmp(argv[1], "on"))
		return lb_do_cmd(LIGHTBAR_CMD_ON, &param, &resp);

	if (!strcasecmp(argv[1], "version")) {
		r = lb_do_cmd(LIGHTBAR_CMD_VERSION, &param, &resp);
		if (!r)
			printf("version %d flags 0x%x\n",
			       resp.version.num, resp.version.flags);
		return r;
	}

	if (argc > 1 && !strcasecmp(argv[1], "brightness")) {
		char *e;
		int rv;
		if (argc > 2) {
			param.set_brightness.num = 0xff &
				strtoul(argv[2], &e, 16);
			return lb_do_cmd(LIGHTBAR_CMD_SET_BRIGHTNESS,
					 &param, &resp);
		}
		rv = lb_do_cmd(LIGHTBAR_CMD_GET_BRIGHTNESS,
			       &param, &resp);
		if (rv)
			return rv;
		printf("%02x\n", resp.get_brightness.num);
		return 0;
	}

	if (argc > 1 && !strcasecmp(argv[1], "demo")) {
		int rv;
		if (argc > 2) {
			if (!strcasecmp(argv[2], "on") || argv[2][0] == '1')
				param.demo.num = 1;
			else if (!strcasecmp(argv[2], "off") ||
				 argv[2][0] == '0')
				param.demo.num = 0;
			else {
				fprintf(stderr, "Invalid arg\n");
				return -1;
			}
			return lb_do_cmd(LIGHTBAR_CMD_DEMO, &param, &resp);
		}

		rv = lb_do_cmd(LIGHTBAR_CMD_GET_DEMO, &param, &resp);
		if (rv)
			return rv;
		printf("%s\n", resp.get_demo.num ? "on" : "off");
		return 0;
	}

	if (argc > 2 && !strcasecmp(argv[1], "seq")) {
		char *e;
		uint8_t num;
		num = 0xff & strtoul(argv[2], &e, 16);
		if (e && *e) {
			if (!strcasecmp(argv[2], "stop"))
				num = 0x8;
			else if (!strcasecmp(argv[2], "run"))
				num = 0x9;
			else if (!strcasecmp(argv[2], "konami"))
				num = 0xA;
			else
				num = LIGHTBAR_NUM_SEQUENCES;
		}
		if (num >= LIGHTBAR_NUM_SEQUENCES) {
			fprintf(stderr, "Invalid arg\n");
			return -1;
		}
		param.seq.num = num;
		return lb_do_cmd(LIGHTBAR_CMD_SEQ, &param, &resp);
	}

	if (argc == 4) {
		char *e;
		param.reg.ctrl = 0xff & strtoul(argv[1], &e, 16);
		param.reg.reg = 0xff & strtoul(argv[2], &e, 16);
		param.reg.value = 0xff & strtoul(argv[3], &e, 16);
		return lb_do_cmd(LIGHTBAR_CMD_REG, &param, &resp);
	}

	if (argc == 5) {
		char *e;
		param.set_rgb.led = strtoul(argv[1], &e, 16);
		param.set_rgb.red = strtoul(argv[2], &e, 16);
		param.set_rgb.green = strtoul(argv[3], &e, 16);
		param.set_rgb.blue = strtoul(argv[4], &e, 16);
		return lb_do_cmd(LIGHTBAR_CMD_SET_RGB, &param, &resp);
	}

	/* Only thing left is to try to read an LED value */
	if (argc == 2) {
		char *e;
		param.get_rgb.led = strtoul(argv[1], &e, 0);
		if (!(e && *e)) {
			r = lb_do_cmd(LIGHTBAR_CMD_GET_RGB, &param, &resp);
			if (r)
				return r;
			printf("%02x %02x %02x\n",
			       resp.get_rgb.red,
			       resp.get_rgb.green,
			       resp.get_rgb.blue);
			return 0;
		}
	}

	return 0;
}

#define PD_ROLE_SINK   0
#define PD_ROLE_SOURCE 1
#define PD_ROLE_UFP    0
#define PD_ROLE_DFP    1

static int cmd_ec_usbpd(int argc, const char **argv)
{
	const char *role_str[] = {"", "toggle", "toggle-off", "sink", "source"};
	const char *mux_str[] = {"", "none", "usb", "dp", "dock", "auto"};
	struct ec_params_usb_pd_control p;
	struct ec_response_usb_pd_control_v1 r;
	int rv, i;
        unsigned j;
	int option_ok;
	char *e;

	if (!get_ec())
		return -ENODEV;

	p.role = USB_PD_CTRL_ROLE_NO_CHANGE;
	p.mux = USB_PD_CTRL_MUX_NO_CHANGE;

	if (argc < 2) {
		fprintf(stderr, "No port specified.\n");
		return -1;
	}

	p.port = strtol(argv[1], &e, 0);
	if (e && *e) {
		fprintf(stderr, "Invalid param (port)\n");
		return -1;
	}

	for (i = 2; i < argc; ++i) {
		option_ok = 0;
		if (!strcmp(argv[i], "auto")) {
			if (argc != 3) {
				fprintf(stderr, "\"auto\" may not be used "
						"with other options.\n");
				return -1;
			}
			p.role = USB_PD_CTRL_ROLE_TOGGLE_ON;
			p.mux = USB_PD_CTRL_MUX_AUTO;
			continue;
		}

		for (j = 0; j < ARRAY_SIZE(role_str); ++j) {
			if (!strcmp(argv[i], role_str[j])) {
				if (p.role != USB_PD_CTRL_ROLE_NO_CHANGE) {
					fprintf(stderr,
						"Only one role allowed.\n");
					return -1;
				}
				p.role = j;
				option_ok = 1;
				break;
			}
		}
		if (option_ok)
			continue;

		for (j = 0; j < ARRAY_SIZE(mux_str); ++j) {
			if (!strcmp(argv[i], mux_str[j])) {
				if (p.mux != USB_PD_CTRL_MUX_NO_CHANGE) {
					fprintf(stderr,
						"Only one mux type allowed.\n");
					return -1;
				}
				p.mux = j;
				option_ok = 1;
				break;
			}
		}

		if (!option_ok) {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			return -1;
		}
	}

	rv = flash_cmd(ec, EC_CMD_USB_PD_CONTROL, 1, &p, sizeof(p),
			&r, sizeof(r));

	if (rv < 0 || argc != 2)
		return (rv < 0) ? rv : 0;

	printf("Port C%d is %s,%s, Role:%s %s Polarity:CC%d State:%s\n",
	       p.port, (r.enabled & 1) ? "enabled" : "disabled",
	       (r.enabled & 2) ? "connected" : "disconnected",
	       r.role & PD_ROLE_SOURCE ? "SRC" : "SNK",
	       r.role & (PD_ROLE_DFP << 1) ? "DFP" : "UFP",
	       r.polarity + 1, r.state);

	return (rv < 0 ? rv : 0);
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

static int cmd_ec_version(int argc, const char **argv)
{
        static const char * const image_names[] = {"unknown", "RO", "RW"};
	struct ec_response_get_version r;
	char build_string[128];
	int rv;

	if (!get_ec())
		return -ENODEV;

	rv = flash_cmd(ec, EC_CMD_GET_VERSION, 0, NULL, 0, &r, sizeof(r));
	if (rv < 0) {
		fprintf(stderr, "ERROR: EC_CMD_GET_VERSION failed: %d\n", rv);
		return rv;
	}
	rv = flash_cmd(ec, EC_CMD_GET_BUILD_INFO, 0,
			NULL, 0, build_string, sizeof(build_string));
	if (rv < 0) {
		fprintf(stderr, "ERROR: EC_CMD_GET_BUILD_INFO failed: %d\n",
				rv);
		return rv;
	}

	/* Ensure versions are null-terminated before we print them */
	r.version_string_ro[sizeof(r.version_string_ro) - 1] = '\0';
	r.version_string_rw[sizeof(r.version_string_rw) - 1] = '\0';
	build_string[sizeof(build_string) - 1] = '\0';

	/* Print versions */
	printf("RO version:    %s\n", r.version_string_ro);
	printf("RW version:    %s\n", r.version_string_rw);
	printf("Firmware copy: %s\n",
	       (r.current_image < ARRAY_SIZE(image_names) ?
		image_names[r.current_image] : "?"));
	printf("Build info:    %s\n", build_string);

	return 0;
}

struct command subcmds_ec[] = {
	CMD(ec_battery, "Show battery status"),
	CMD(ec_gpioget, "Get the value of GPIO signal"),
	CMD(ec_gpioset, "Set the value of GPIO signal"),
	CMD(ec_lightbar, "Lightbar control commands"),
	CMD(ec_usbpd, "Control USB PD/type-C"),
	CMD(ec_usbpdpower, "Power information about USB PD ports"),
	CMD(ec_version, "Prints EC version"),
	CMD_GUARD_LAST
};
