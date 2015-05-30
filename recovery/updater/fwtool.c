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
/* Command-Line utility to exercise firmware interfaces */

#define LOG_TAG "fwtool"

#include <cutils/log.h>

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "edify/expr.h"
#include "debug_cmd.h"
#include "flash_device.h"
#include "fmap.h"
#include "update_fw.h"
#include "vboot_interface.h"

static void *spi;
static void *ec;

static void *get_spi(void)
{
	if (!spi)
		spi = flash_open("spi", NULL);

	return spi;
}

static void *get_ec(void)
{
	if (!ec)
		ec = flash_open("ec", NULL);

	return ec;
}

static void dump_fmap(struct flash_device *dev)
{
	int i;
	struct fmap *fmap;

	fmap = flash_get_fmap(dev);
	if (!fmap)
		return;

	printf("FMAP '%s' ver %d.%d base 0x%" PRIx64 " size 0x%x\n",
		fmap->name, fmap->ver_major, fmap->ver_minor,
		fmap->base, fmap->size);
	for (i = 0; i < fmap->nareas; i++) {
		struct fmap_area *a = fmap->areas+i;
		printf("%16s @%08x size 0x%08x %2s %s\n",
			a->name, a->offset, a->size,
			a->flags & FMAP_AREA_RO ? "RO" : "",
			a->flags & FMAP_AREA_STATIC ? "static" : "");
	}
}

static void dump_section(struct flash_device *dev, const char *name)
{
	size_t size;
	off_t offset;
	char *content;

	content = fmap_read_section(dev, name, &size, &offset);
	if (content) {
		content[size - 1] = '\0';
		printf("[%s]@%lx={%s}\n", name, offset, content);
	}
}

static int cmd_flash_fmap(int argc, const char **argv)
{
	if (!get_spi())
		return -ENODEV;

	dump_fmap(spi);
	dump_section(spi, "RO_FRID");
	dump_section(spi, "RW_FWID_A");
	dump_section(spi, "RW_FWID_B");
	return 0;
}

static int cmd_vboot(int argc, const char **argv)
{
	char *hwid = fdt_read_string("hardware-id");
	char *version = fdt_read_string("firmware-version");
	char *ro_version = fdt_read_string("readonly-firmware-version");
	char *type = fdt_read_string("firmware-type");
	char *ec = fdt_read_string("active-ec-firmware");
	printf("HWID: %s\n", hwid);
	printf("Version: %s\n", version);
	printf("RO Version: %s\n", ro_version);
	printf("FW Type: %s\n", type);
	printf("EC: %s\n", ec);
	printf("FW partition: %c\n", vboot_get_mainfw_act());
	free(hwid);
	free(version);
	free(ro_version);
	free(type);
	free(ec);

	return 0;
}

static int cmd_update(int argc, const char **argv)
{
	Value mainv, ecv;
	if (argc < 2)
		return -EINVAL;

	printf("Updating using images %s/%s ...\n", argv[0], argv[1]);
	mainv.type = VAL_STRING;
	mainv.data = (void *)argv[0];
	ecv.type = VAL_STRING;
	ecv.data = (void *)argv[1];
	update_fw(&mainv, &ecv, 1);
	printf("Done.\n");

	return -ENOENT;
}

static struct command subcmds_flash[] = {
	CMD(flash_fmap, "Dump FMAP information"),
	CMD_GUARD_LAST
};

static struct command cmds[] = {
	SUBCMDS(ec,    "Send commands directly to the EC"),
	SUBCMDS(flash, "Read/Write/Dump flash"),
	CMD(update,    "Update the firmwares"),
	CMD(vboot,     "dump VBoot information"),
	CMD_GUARD_LAST
};

static void print_usage(struct command *commands, int idx, int prefix,
			int argc, const char **argv)
{
	int i;
	struct command *c = commands;
	fprintf(stderr, "Usage: ");
	for (i = 0; i <= idx; i++)
		fprintf(stderr,"%s ", argv[i]);
	fprintf(stderr, "\n");
	while (c->name) {
		fprintf(stderr, "\t\t%-12s: %s\n", c->name + prefix, c->help);
		c++;
	}
}

static int run_cmd(struct command *commands, int idx, int prefix,
		   int argc, const char **argv)
{
	struct command *c = commands;
	if (argc <= idx + 1)
		goto no_cmd;

	idx += 1;
	while (c->name) {
		if (!strcmp(c->name + prefix, argv[idx])) {
			int nprefx = prefix + strlen(c->name) + 1;
			if (argc > 1 && c->subcmd)
				return run_cmd(c->subcmd, idx, nprefx,
						argc, argv);
			else if (c->handler)
				return c->handler(argc - idx, argv + idx);
			else
				print_usage(c->subcmd, idx, nprefx, argc, argv);
			return -EINVAL;
		}
		c++;
	}
	idx -= 1; /* last command word was unknown */
no_cmd:
	print_usage(commands, idx, prefix, argc, argv);
	return -ENOENT;
}

int main(int argc, const char **argv)
{
	int res = -EINVAL;

	printf("Firmware debug Tool\n");

	res = run_cmd(cmds, 0, 0, argc, argv);

	/* Clean up our flash handlers */
	if (spi)
		flash_close(spi);
	if (ec)
		flash_close(ec);

	return res;
}
