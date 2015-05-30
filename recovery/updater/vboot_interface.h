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
#ifndef _RECOVERY_VBOOT_INTERFACE_H_
#define _RECOVERY_VBOOT_INTERFACE_H_

char *fdt_read_string(const char *prop);
uint32_t fdt_read_u32(const char *prop);

char vboot_get_mainfw_act(void);

off_t fmap_scan_offset(struct flash_device *dev, off_t end);
struct fmap *fmap_load(struct flash_device *dev, off_t offset);
void *fmap_read_section(struct flash_device *dev,
			const char *name, size_t *size, off_t *offset);

uint8_t *gbb_get_rootkey(struct flash_device *dev, size_t *size);

int vbnv_set_fw_try_next(struct flash_device *ec, int next);

#endif /* _RECOVERY_VBOOT_INTERFACE_H_ */
