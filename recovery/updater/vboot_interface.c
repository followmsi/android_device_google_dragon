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
/* Vboot/crossystem interface */

#define LOG_TAG "fwtool"

#include <cutils/log.h>

#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ec_commands.h"
#include "flash_device.h"
#include "fmap.h"
#include "vboot_struct.h"
#include "gbb_header.h"

/* ---- VBoot information passed by the firmware through the device-tree ---- */

/* Base name for firmware FDT files */
#define FDT_BASE_PATH "/proc/device-tree/firmware/chromeos"

char *fdt_read_string(const char *prop)
{
	char filename[PATH_MAX];
	FILE *file;
	size_t size;
	char *data;

	snprintf(filename, sizeof(filename), FDT_BASE_PATH "/%s", prop);
	file = fopen(filename, "r");
	if (!file) {
		ALOGD("Unable to open FDT property %s\n", prop);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	data = malloc(size + 1);
	if (!data)
		return NULL;
	data[size] = '\0';

	rewind(file);
	if (fread(data, 1, size, file) != size) {
		ALOGD("Unable to read FDT property %s\n", prop);
		return NULL;
	}
	fclose(file);

	return data;
}

uint32_t fdt_read_u32(const char *prop)
{
	char filename[PATH_MAX];
	FILE *file;
	int data = 0;

	snprintf(filename, sizeof(filename), FDT_BASE_PATH "/%s", prop);
	file = fopen(filename, "r");
	if (!file) {
		ALOGD("Unable to open FDT property %s\n", prop);
		return -1U;
	}
	if (fread(&data, 1, sizeof(data), file) != sizeof(data)) {
		ALOGD("Unable to read FDT property %s\n", prop);
		return -1U;
	}
	fclose(file);

	return ntohl(data); /* FDT is network byte order */
}

char vboot_get_mainfw_act(void)
{
	VbSharedDataHeader *shd = (void *)fdt_read_string("vboot-shared-data");
	char v;

	if (!shd || shd->magic != VB_SHARED_DATA_MAGIC) {
		ALOGD("Cannot retrieve VBoot shared data\n");
		if (shd)
			free(shd);
		return 'E'; /* Error */
	}

	switch(shd->firmware_index) {
	case 0:
		v = 'A'; /* RW_A in use */
		break;
	case 1:
		v = 'B'; /* RW_B in use */
		break;
	case 0xFF:
		v = 'R'; /* Recovery/RO in use */
		break;
	default:
		ALOGD("Invalid firmware index : %02x\n", shd->firmware_index);
		v = 'E'; /* Error */
	}

	free(shd);
	return v;
}

/* ---- Flash Maps handling ---- */

off_t fmap_scan_offset(struct flash_device *dev, off_t end)
{
	struct fmap h;
	uint32_t off = end - (end % 64); /* start on a 64-byte boundary */
	int res;

	/*
	 * Try to find the FMAP signature at 64-byte boundaries
         * starting from the end.
	 */
	do {
		off -= 64;
		res = flash_read(dev, off, &h, sizeof(h.signature));
		if (res)
			break;
		if (!memcmp(&h.signature, FMAP_SIGNATURE, sizeof(h.signature)))
			break;
	} while (off);

	return off;
}

struct fmap *fmap_load(struct flash_device *dev, off_t offset)
{
	struct fmap hdr;
	struct fmap *fmap;
	size_t size;
	int res;

	ALOGD("Searching FMAP @0x%08lx\n", offset);
	res = flash_read(dev, offset, &hdr, sizeof(hdr));
	if (res) {
		ALOGD("Cannot read FMAP header\n");
		return NULL;
	}

	if (memcmp(&hdr.signature, FMAP_SIGNATURE, sizeof(hdr.signature))) {
		ALOGD("Cannot find FMAP\n");
		return NULL;
	}

	size = sizeof(struct fmap) + hdr.nareas * sizeof(struct fmap_area);
	fmap = malloc(size);

	res = flash_read(dev, offset, fmap, size);
	if (res) {
		ALOGD("Cannot read FMAP\n");
		free(fmap);
		return NULL;
	}

	return fmap;
}

void *fmap_read_section(struct flash_device *dev,
			const char *name, size_t *size, off_t *offset)
{
	int i, r;
	struct fmap *fmap = flash_get_fmap(dev);
	void *data;

	if (!fmap)
		return NULL;

	for (i = 0; i < fmap->nareas; i++)
		if (!strcmp(name, (const char*)fmap->areas[i].name))
			break;
	if (i == fmap->nareas) {
		ALOGD("Cannot find section '%s'\n", name);
		return NULL;
	}
	*size = fmap->areas[i].size;
	data = malloc(*size);
	if (!data)
		return NULL;

	r = flash_read(dev, fmap->areas[i].offset, data, *size);
	if (r) {
		ALOGD("Cannot read section '%s'\n", name);
		free(data);
		return NULL;
	}
	if (offset)
		*offset = fmap->areas[i].offset;

	return data;
}

/* ---- Google Binary Block (GBB) ---- */

uint8_t *gbb_get_rootkey(struct flash_device *dev, size_t *size)
{
	size_t gbb_size;
	uint8_t *gbb = flash_get_gbb(dev, &gbb_size);
	GoogleBinaryBlockHeader *hdr = (void *)gbb;

	if (!gbb || memcmp(hdr->signature, GBB_SIGNATURE, GBB_SIGNATURE_SIZE) ||
	    gbb_size < sizeof(*hdr))
		return NULL;

	if (hdr->rootkey_offset + hdr->rootkey_size > gbb_size)
		return NULL;

	if (size)
		*size = hdr->rootkey_size;

	return gbb + hdr->rootkey_offset;
}

/* ---- VBoot NVRAM (stored by cros_ec) ---- */

/* bits definition in NVRAM */
#define BOOT2_OFFSET                       7
#define BOOT2_RESULT_MASK               0x03
#define BOOT2_TRIED                     0x04
#define BOOT2_TRY_NEXT                  0x08
#define CRC_OFFSET                        15

static uint8_t crc8(const uint8_t *data, int len)
{
	uint32_t crc = 0;
	int i, j;

	for (j = len; j; j--, data++) {
		crc ^= (*data << 8);
		for(i = 8; i; i--) {
			if (crc & 0x8000)
				crc ^= (0x1070 << 3);
			crc <<= 1;
		}
	}

	return (uint8_t)(crc >> 8);
}

#define VB2_NVDATA_SIZE	16

int vbnv_readwrite(struct flash_device *spi, int off, uint8_t mask,
		   uint8_t *value, int write)
{
	int res;
	size_t size;
	off_t offset;
	uint8_t *block, *nvram, *end, *curr;
	uint8_t dummy[VB2_NVDATA_SIZE];

	if (off >= VB2_NVDATA_SIZE) {
		ALOGW("ERROR: Incorrect offset %d for NvStorage\n", off);
		return -EIO;
	}

	/* Read NVRAM. */
	nvram = fmap_read_section(spi, "RW_NVRAM", &size, &offset);
	/*
	 * Ensure NVRAM is found, size is atleast 1 block and total size is
	 * multiple of VB2_NVDATA_SIZE.
	 */
	if ((nvram == NULL) || (size < VB2_NVDATA_SIZE) ||
	    (size % VB2_NVDATA_SIZE)) {
		ALOGW("ERROR: NVRAM not found\n");
		return -EIO;
	}

	/* Create an empty dummy block to compare. */
	memset(dummy, 0xFF, sizeof(dummy));

	/*
	 * Loop until the last used block in NVRAM.
	 * 1. All blocks will not be empty since we just booted up fine.
	 * 2. If all blocks are used, select the last block.
	 */
	block = nvram;
	end = block + size;
	for (curr = block; curr < end; curr += VB2_NVDATA_SIZE) {
		if (memcmp(curr, dummy, VB2_NVDATA_SIZE) == 0)
			break;
		block = curr;
	}

	if (write) {
		block[off] = (block[off] & ~mask) | (*value & mask);
		block[CRC_OFFSET] = crc8(block, CRC_OFFSET);

		if (flash_erase(spi, offset, size)) {
			ALOGW("ERROR: Cannot erase flash\n");
			return -EIO;
		}

		if (flash_write(spi, offset, nvram, size)) {
			ALOGW("ERROR: Cannot update NVRAM\n");
			return -EIO;
		}

		ALOGD("NVRAM updated.\n");
	} else {
		*value = block[off] & mask;
	}

	return 0;
}

int vbnv_set_fw_try_next(struct flash_device *spi, int next)
{
	uint8_t value = next ? BOOT2_TRY_NEXT : 0;
	return vbnv_readwrite(spi, BOOT2_OFFSET, BOOT2_TRY_NEXT,
			      &value, 1);
}

/* ---- Vital Product Data handling ---- */
