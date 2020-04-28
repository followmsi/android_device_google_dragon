/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#define BTM_DEF_LOCAL_NAME "Pixel C"

#define BTA_DM_COD {0x1A, 0x01, 0x1C}

// Detect disconnects faster (N x 10ms)
#define BTM_BLE_CONN_TIMEOUT_DEF 300

#define BTIF_HF_SERVICES (BTA_HSP_SERVICE_MASK)

// #define BLE_VND_INCLUDED TRUE Disable BLE_VND_INCLUDED for now If enabled, crashes the stack when bluetooth is turned on. Change-Id: I478970f153f8d8d6af44de6e0b21e68b4cb33a37
// #define BTM_BLE_ADV_TX_POWER {-21, -15, -7, 1, 9} Removed unused or unnecessary defines 56c48f06db8438ff66e6461823e50398e92826d6
// #define PAN_SUPPORTS_ROLE_NAP FALSE
#endif
