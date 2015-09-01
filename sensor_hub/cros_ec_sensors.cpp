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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#define LOG_TAG "CrosECSensor"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <utils/Timers.h>

#include "cros_ec_sensors.h"

/*****************************************************************************/
static int min(int a, int b) {
    return (a < b) ? a : b;
}

/*
 * sysfs_set_input_attr: Helper function to write a sysfs attribute.
 */
int CrosECSensor::sysfs_set_input_attr(const char *path, const char *attr,
                                       const char *value, size_t len)
{
    char fname[PATH_MAX];
    int fd;
    int rc;

    snprintf(fname, sizeof(fname), "%s%s/%s", IIO_DIR, path, attr);
    fname[sizeof(fname) - 1] = '\0';

    fd = open(fname, O_WRONLY);
    if (fd < 0) {
        ALOGE("%s: fname = %s, fd = %d, failed: %s\n", __func__,
              fname, fd, strerror(errno));
        return -EACCES;
    }

    rc = write(fd, value, (size_t)len);
    if (rc < 0)
        ALOGE("%s: write failed: fd = %d, rc = %d, strerr = %s\n", __func__,
              fd, rc, strerror(errno));

    close(fd);

    return rc < 0 ? rc : 0;
}

int CrosECSensor::sysfs_set_input_attr_by_int(const char *path,
                                              const char *attr, int value)
{
    char buf[INT32_CHAR_LEN];

    size_t n = snprintf(buf, sizeof(buf), "%d", value);
    if (n > sizeof(buf)) {
        return -1;
    }

    return sysfs_set_input_attr(path, attr, buf, n);
}

/*
 * Constructor.
 *
 * Setup and open the ring buffer.
 */
CrosECSensor::CrosECSensor(struct cros_ec_sensor_info *sensor_info,
        const char *ring_device_name,
        const char *trigger_name)
    : mSensorInfo(sensor_info)
{
    char ring_buffer_name[IIO_MAX_NAME_LENGTH] = "/dev/";

    strcat(ring_buffer_name, ring_device_name);
    mDataFd = open(ring_buffer_name, O_RDONLY);
    if (mDataFd < 0) {
        ALOGE("open file '%s' failed: %s\n",
                ring_buffer_name, strerror(errno));
    }

    strcpy(mRingPath, ring_device_name);

    if (sysfs_set_input_attr_by_int(mRingPath, "buffer/enable", 0) < 0) {
        ALOGE("disable IIO buffer failed: %s\n", strerror(errno));
        return;
    }
    if (sysfs_set_input_attr(mRingPath, "trigger/current_trigger",
                trigger_name, strlen(trigger_name))) {
        ALOGE("Unable to set trigger name: %s\n", strerror(errno));
        return;
    }
    if (sysfs_set_input_attr_by_int(mRingPath, "buffer/length",
                IIO_MAX_BUFF_SIZE) < 0) {
        ALOGE("set IIO buffer length (%d) failed: %s\n",
                IIO_MAX_BUFF_SIZE, strerror(errno));
    }
    if (sysfs_set_input_attr_by_int(mRingPath, "buffer/enable", 1) < 0) {
        ALOGE("enable IIO buffer failed: %s\n",
                strerror(errno));
        return;
    }
}

CrosECSensor::~CrosECSensor() {
    close(mDataFd);
}

/*
 * getFd: retrieve the ring file descriptor.
 *
 * Needed for CrosECSensor creator to listen to the buffer.
 */
int CrosECSensor::getFd(void)
{
    return mDataFd;
}

/*
 * flush: Flush entry point.
 *
 * Issue the flush for a particular sensor to the EC via iio.
 */
int CrosECSensor::flush(int handle)
{
    struct cros_ec_sensor_info *info = &mSensorInfo[handle];

    if (!info->enabled)
        return -EINVAL;

    return sysfs_set_input_attr_by_int(info->device_name, "flush", 1);
}

/*
 * activate: Activate entry point.
 *
 * When enabled set the sensor frequency. If not enabled, set
 * the sensor in suspend mode by setting the frequency to 0.
 */
int CrosECSensor::activate(int handle, int enabled)
{
    struct cros_ec_sensor_info *info = &mSensorInfo[handle];
    int err;
    /*
     * Frequency is in mHz, sampling period in ns, use 10^(9 + 3)
     * coefficient.
     */
    long frequency = enabled ? 1e12 / info->sampling_period_ns : 0;

    err = sysfs_set_input_attr_by_int(info->device_name,
            "frequency", frequency);
    if (err)
        return err;

    long ec_period = nanoseconds_to_milliseconds(info->max_report_latency_ns);

    if (enabled)
        ec_period = min(CROS_EC_MAX_SAMPLING_PERIOD, ec_period);
    else
        ec_period = 0;

    /* Sampling is encoded on a 16bit so, so the maximal period is ~65s. */
    err = sysfs_set_input_attr_by_int(
            info->device_name, "sampling_frequency", ec_period);

    if (!err)
        info->enabled = enabled;
    return err;
}

/*
 * batch: Batch entry point.
 *
 * Set the EC sampling frequency. Check boundaries to prevent polling too fast.
 */
int CrosECSensor::batch(int handle,
        int64_t sampling_period_ns,
        int64_t max_report_latency_ns)
{
    struct cros_ec_sensor_info *info = &mSensorInfo[handle];

    info->max_report_latency_ns = max_report_latency_ns;

    if (nanoseconds_to_microseconds(sampling_period_ns) >
        info->sensor_data.maxDelay)
        info->sampling_period_ns = microseconds_to_nanoseconds(info->sensor_data.maxDelay);
    else if (nanoseconds_to_microseconds(sampling_period_ns) <
             info->sensor_data.minDelay)
        info->sampling_period_ns = microseconds_to_nanoseconds(info->sensor_data.minDelay);
    else
        info->sampling_period_ns = sampling_period_ns;

    /*
     * Note that the sensor hub limit minimal sampling frequency at few ms.
     * Which is good, because HAL shold not ask for polling sensor at
     * more than the sampling period, set in sensor_t.
     */
    if (info->max_report_latency_ns < info->sampling_period_ns) {
        /*
         * We have to report an event as soon as available.
         * Set polling frequency as low as sampling frequency
         */
        info->max_report_latency_ns = info->sampling_period_ns;
    }

    /* Call activate to change the paramters if necessary */
    return activate(handle, info->enabled);
}

/*
 * readEvents: Read events from the iio ring buffer.
 *
 * data: where to put the events.
 * count: maximal number of events to read from iio.
 * If iio indicates no more events are available, return.
 */
int CrosECSensor::readEvents(sensors_event_t* data, int count)
{
    int rc;

    if (count < 1) {
        return -EINVAL;
    }

    /*
     * Do a single read to collects all pending events.
     * up to what poll caller can handle.
     */
    rc = read(mDataFd, mEvents, sizeof(cros_ec_event) * count);
    if (rc < 0) {
        ALOGE("rc %d while reading ring\n", rc);
        return rc;
    }
    if (rc % sizeof(cros_ec_event) != 0) {
        ALOGE("Incomplete event while reading ring: %d\n", rc);
        return -EINVAL;
    }

    int nb_events = rc / sizeof(cros_ec_event);
    int data_events = 0;
    for (int i = 0; i < nb_events; i++) {
        rc = processEvent(data, &mEvents[i]);
        if (rc == 0) {
            data++;
            data_events++;
        }
    }

    return data_events;
}

/*
 * processEvent:
 *
 * Internal function to translate an event from the iio ring
 * buffer into a sensors_event_t.
 *
 * Support flush meta event and regular events.
 */
int CrosECSensor::processEvent(sensors_event_t* data, const cros_ec_event *event)
{
    struct cros_ec_sensor_info *info = &mSensorInfo[event->sensor_id];

    if (event->flags & CROS_EC_EVENT_FLUSH_FLAG) {
        data->version = META_DATA_VERSION;
        data->sensor = 0;
        data->type = SENSOR_TYPE_META_DATA;
        data->reserved0 = 0;
        data->timestamp = 0;
        data->meta_data.what = META_DATA_FLUSH_COMPLETE;
        data->meta_data.sensor = event->sensor_id;
        return 0;
    }

    /*
     * The sensor hub can send data even if the sensor is not set up.
     * workaround it unitl b/23238991 is fixed.
     */
    if (!info->enabled)
        return -ENOKEY;

    data->version = sizeof(sensors_event_t);
    data->sensor = event->sensor_id;
    data->type = info->sensor_data.type;
    data->timestamp = event->timestamp;
    data->acceleration.status = SENSOR_STATUS_ACCURACY_LOW;

    /*
     * Even for sensor with one axis (light, proxmity), be sure to write
     * the other vectors. EC 0s them out.
     */
    float d;
    for (int i = X ; i < MAX_AXIS; i++) {
        switch (info->sensor_data.type) {
        case SENSOR_TYPE_ACCELEROMETER:
        case SENSOR_TYPE_GYROSCOPE:
        case SENSOR_TYPE_MAGNETIC_FIELD:
            d = event->vector[i];
            break;
        case SENSOR_TYPE_LIGHT:
        case SENSOR_TYPE_PROXIMITY:
            d = (uint16_t)event->vector[i];
            break;
        default:
            return -EINVAL;
        }
        data->acceleration.v[i] = d * mSensorInfo[event->sensor_id].sensor_data.resolution;
    }
    return 0;
}
