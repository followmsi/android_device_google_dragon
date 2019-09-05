/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <string.h>

#include "common.h"
#include "recovery_ui/device.h"
#include "flash_device.h"
#include "recovery_ui/screen_ui.h"
#include "recovery_ui/ui.h"
#include "vboot_interface.h"

class DragonDevice : public Device {
  public:
    DragonDevice(RecoveryUI* ui) : Device(ui) { }

    virtual bool PostWipeData() {
        if (reason) {
            struct flash_device *spi = flash_open("spi", NULL);

            if (spi == NULL) {
                return true;
            }

            if (!strcmp(reason, "fastboot_oem_unlock")) {
                vbnv_set_flag(spi, "dev_boot_fastboot_full_cap", 0x1);
                vbnv_set_flag(spi, "recovery_reason", 0xC3);
            } else if (!strcmp(reason, "fastboot_oem_lock")) {
                vbnv_set_flag(spi, "dev_boot_fastboot_full_cap", 0x0);
                vbnv_set_flag(spi, "recovery_reason", 0xC3);
            }

            flash_close(spi);
        }

        return true;
    };
};


Device* make_device() {
    return new DragonDevice(new ScreenRecoveryUI);
}
