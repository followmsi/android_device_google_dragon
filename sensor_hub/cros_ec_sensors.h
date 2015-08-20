/*
 * Copyright (C) 2008-2014 The Android Open Source Project
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

#ifndef CROS_EC_SENSORS_H
#define CROS_EC_SENSORS_H

#include <errno.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <utils/BitSet.h>

#include <hardware/sensors.h>

#define IIO_DIR  "/sys/bus/iio/devices/"
#define IIO_MAX_NAME_LENGTH 30
#define IIO_MAX_BUFF_SIZE 512
#define INT32_CHAR_LEN 12

#define IIO_MAX_DEVICE_NAME_LENGTH (strlen(IIO_DIR) + IIO_MAX_NAME_LENGTH)

enum {X, Y, Z, MAX_AXIS};

extern const char *cros_ec_sensor_names[];

struct cros_ec_event {
    uint8_t sensor_id;
    uint8_t flags;
    int16_t vector[MAX_AXIS];
    uint64_t timestamp;
} __packed;

#define CROS_EC_EVENT_FLUSH_FLAG 0x1

struct cros_ec_sensor_info {
    /* description of the sensor, as reported to sensorservice. */
    sensor_t sensor_data;
    const char *device_name;
    int64_t sampling_period_ns;
    int64_t max_report_latency_ns;
    bool enabled;
};

class CrosECSensor {
    struct cros_ec_sensor_info *mSensorInfo;
    char mRingPath[IIO_MAX_DEVICE_NAME_LENGTH];
    cros_ec_event mEvents[IIO_MAX_BUFF_SIZE];
    int mDataFd;

    int sysfs_set_input_attr(const char *path, const char *attr, const char *value, size_t len);
    int sysfs_set_input_attr_by_int(const char *path, const char *attr, int value);
    int processEvent(sensors_event_t* data, const cros_ec_event *event);
public:
    CrosECSensor(struct cros_ec_sensor_info *sensor_info,
        const char *ring_device_name,
        const char *trigger_name);
    virtual ~CrosECSensor();
    virtual int getFd(void);
    int readEvents(sensors_event_t* data, int count);

    virtual int activate(int handle, int enabled);
    virtual int batch(int handle, int64_t period_ns, int64_t timeout);
    virtual int flush(int handle);
};

#endif  // CROS_EC_SENSORS_H
