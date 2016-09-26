/*
 * Copyright 2013 The Android Open Source Project
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

#include <cutils/properties.h>
#include <dumpstate.h>

void dumpstate_board()
{
    Dumpstate& ds = Dumpstate::GetInstance();

    /* ask init.dragon.rc to dump the charging state and wait */
    property_set("debug.bq25892", "dump");
    sleep(1);

    ds.DumpFile("EC Version", "/sys/class/chromeos/cros_ec/version");
    ds.RunCommand("FW Version", {"fwtool", "vboot"}, CommandOptions::WithTimeout(5).Build());
    ds.DumpFile("Charger chip registers", "/data/misc/fw_logs/bq25892.txt");
    ds.DumpFile("Battery gas gauge", "/sys/class/power_supply/bq27742-0/uevent");
    ds.DumpFile("Touchscreen firmware updater", "/data/misc/touchfwup/rmi4update.txt");
    ds.DumpFile("Ion heap", "/d/ion/heaps/system");
};
