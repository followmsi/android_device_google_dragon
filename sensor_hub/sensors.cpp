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

#define LOG_TAG "Sensors"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#include <utils/Atomic.h>
#include <utils/Log.h>

#include <hardware/sensors.h>

#include "cros_ec_sensors.h"

/*****************************************************************************/

/*****************************************************************************/

#define UNSET_FIELD -1

enum cros_ec_sensor_device {
    CROS_EC_ACCEL,
    CROS_EC_GYRO,
    CROS_EC_MAG,
    CROS_EC_PROX,
    CROS_EC_LIGHT,
    CROS_EC_RING,
    CROS_EC_MAX_DEVICE,
};

/* Name of iio devices, as reported by cros_ec_dev.c */
const char *cros_ec_sensor_names[] = {
    [CROS_EC_ACCEL] = "cros-ec-accel",
    [CROS_EC_GYRO] = "cros-ec-gyro",
    [CROS_EC_MAG] = "cros-ec-mag",
    [CROS_EC_PROX] = "cros-ec-prox",
    [CROS_EC_LIGHT] = "cros-ec-light",
    [CROS_EC_RING] = "cros-ec-ring",
};

/*
 * Template for sensor_t structure return to motionservice.
 *
 * Some parameters (handle, range, resolution) are retreived
 * from IIO.
 *
 * TODO(gwendal): We could guess the fifo size as well, but
 * we need to issue an ioctl instead of just reading IIO sysfs.
 */
static const struct sensor_t sSensorListTemplate[] = {
    [CROS_EC_ACCEL] = {
        name:               "CrosEC Accelerometer",
        vendor:             "Google",
        version:            1,
        handle:             UNSET_FIELD,
        type:               SENSOR_TYPE_ACCELEROMETER,
        maxRange:           UNSET_FIELD,
        resolution:         UNSET_FIELD,
        power:              0.18f,    /* Based on BMI160 */
        minDelay:           5000,
        fifoReservedEventCount: 0,
        fifoMaxEventCount:  2048,
        stringType:         SENSOR_STRING_TYPE_ACCELEROMETER,
        requiredPermission: 0,
        /*
         * BMI160 has a problem at 6.25Hz or less, FIFO not readable.
         * Works at 12.5Hz, so set maxDelay at 80ms
         */
        maxDelay:           80000,
        flags:              SENSOR_FLAG_CONTINUOUS_MODE,
        reserved:           { 0 }
    },
    [CROS_EC_GYRO] = {
        name:               "CrosEC Gyroscope",
        vendor:             "Google",
        version:            1,
        handle:             UNSET_FIELD,
        type:               SENSOR_TYPE_GYROSCOPE,
        maxRange:           UNSET_FIELD,
        resolution:         UNSET_FIELD,
        power:              0.85f,
        minDelay:           5000,
        fifoReservedEventCount: 0,
        fifoMaxEventCount:  2048,
        stringType:         SENSOR_STRING_TYPE_GYROSCOPE,
        requiredPermission: 0,
        maxDelay:           80000,
        flags:              SENSOR_FLAG_CONTINUOUS_MODE,
        reserved:           { 0 }
    },
    [CROS_EC_MAG] = {
        name:               "CrosEC Compass",
        vendor:             "Google",
        version:            1,
        handle:             UNSET_FIELD,
        type:               SENSOR_TYPE_MAGNETIC_FIELD,
        maxRange:           UNSET_FIELD,
        resolution:         UNSET_FIELD,
        power:              0.5f,  /* Based on BMM150 */
        /*
         * BMI150 uses repetition to reduce output noise.
         * Set ODR at no more than 50Hz.
         */
        minDelay:           20000,
        fifoReservedEventCount: 0,
        fifoMaxEventCount:  2048,
        stringType:         SENSOR_STRING_TYPE_MAGNETIC_FIELD,
        requiredPermission: 0,
        maxDelay:           200000,
        flags:              SENSOR_FLAG_CONTINUOUS_MODE,
        reserved:           { 0 }
    },
    [CROS_EC_PROX] = {
        name:               "CrosEC Proximity",
        vendor:             "Google",
        version:            1,
        handle:             UNSET_FIELD,
        type:               SENSOR_TYPE_PROXIMITY,
        maxRange:           UNSET_FIELD,
        resolution:         UNSET_FIELD,
        power:              0.12f,  /* Based on Si1141 */
        minDelay:           20000,
        fifoReservedEventCount: 0,
        fifoMaxEventCount:  2048,
        stringType:         SENSOR_STRING_TYPE_PROXIMITY,
        requiredPermission: 0,
        /* Forced mode, can be long: 10s */
        maxDelay:           10000000,
        flags:              SENSOR_FLAG_ON_CHANGE_MODE,
        reserved:           { 0 }
    },
    [CROS_EC_LIGHT] = {
        name:               "CrosEC Light",
        vendor:             "Google",
        version:            1,
        handle:             UNSET_FIELD,
        type:               SENSOR_TYPE_LIGHT,
        maxRange:           UNSET_FIELD,
        resolution:         UNSET_FIELD,
        power:              0.12f,  /* Based on Si1141 */
        minDelay:           20000,
        fifoReservedEventCount: 0,
        fifoMaxEventCount:  2048,
        stringType:         SENSOR_STRING_TYPE_LIGHT,
        requiredPermission: 0,
        /* Forced mode, can be long: 10s */
        maxDelay:           10000000,
        flags:              SENSOR_FLAG_ON_CHANGE_MODE,
        reserved:           { 0 }
    },
};

/* We only support the sensors in the lid */
static const char *cros_ec_location = "lid";

static int Stotal_sensor_count_ = 0;
static int Stotal_max_sensor_handle_ = 0;

static struct sensor_t *Ssensor_list_ = NULL;

struct cros_ec_sensor_info *Ssensor_info_ = NULL;

static int cros_ec_open_sensors(const struct hw_module_t *module,
                                const char *id,
                                struct hw_device_t **device);
/*
 * cros_ec_sysfs_read_attr: Helper function to read sysfs attributes.
 *
 * path: the path of the device.
 * attr: attribute to read (path/attr)
 * output: where to put the string read.
 */
static int cros_ec_sysfs_read_attr(const char *path,
                                   const char *attr,
                                   char *output)
{
    char name[IIO_MAX_DEVICE_NAME_LENGTH + 10];
    strcpy(name, path);
    strcat(name, "/");
    strcat(name, attr);
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        ALOGE("Unable to read %s\n", name);
        return -errno;
    }
    int size = read(fd, output, IIO_MAX_NAME_LENGTH);
    close(fd);
    if (size == 0)
        return -EINVAL;
    output[size - 1] = 0;
    ALOGD("Analyzing attribute '%s/%s : %s'\n", path, attr, output);
    return 0;
}

/*
 * cros_ec_get_sensors_list: entry point that returns the list
 * of sensors.
 *
 * At first invocation, build the list from Ssensor_info_,
 * then keep returning the same list.
 *
 * The expected design is a hardcoded list of sensors.
 * Therefore we don't have access to
 */
static int cros_ec_get_sensors_list(struct sensors_module_t*,
        struct sensor_t const** list)
{
    ALOGD("counting sensors: count %d: sensor_list_ %p\n",
            Stotal_sensor_count_, Ssensor_list_);

    if (Stotal_sensor_count_ != 0) {
        *list = Ssensor_list_;
        return Stotal_sensor_count_;
    }

    for (int i = 0 ; i < Stotal_max_sensor_handle_ ; i++) {
        if (Ssensor_info_[i].device_name == NULL)
            continue;

        Stotal_sensor_count_++;
        Ssensor_list_ = (sensor_t*)realloc(Ssensor_list_,
                Stotal_sensor_count_ * sizeof(sensor_t));
        if (Ssensor_list_ == NULL) {
            ALOGI("Unable to allocate Ssensor_list_\n");
            return 0;
        }
        sensor_t *sensor_data;
        sensor_data = &Ssensor_info_[i].sensor_data;
        memcpy(&Ssensor_list_[Stotal_sensor_count_ - 1], sensor_data,
               sizeof(sensor_t));
    }
    *list = Ssensor_list_;
    return Stotal_sensor_count_;
}

/*
 * cros_ec_get_sensors_names: Build list of sensors from IIO
 *
 * Scanning /sys/iio/devices, finds all the sensors managed by the EC.
 *
 * Fill Ssensor_info_ global structure.
 * ring_device_name: name of iio ring buffer. We
 *   will open /dev/<ring_device_name> later
 * ring_trigger_name: Name of hardware trigger for setting the
 *   ring buffer producer side.
 */
static int cros_ec_get_sensors_names(char **ring_device_name,
                                     char **ring_trigger_name)
{
    /*
     * If Ssensor_info_ is valid, we don't want to open
     * the same device twice.
     */
    if (Stotal_max_sensor_handle_ != 0)
        return -EINVAL;

    *ring_device_name = NULL;
    *ring_trigger_name = NULL;

    DIR *iio_dir;
    iio_dir = opendir(IIO_DIR);
    if (iio_dir == NULL) {
        return -ENODEV;
    }
    const struct dirent *ent_device;
    while (ent_device = readdir(iio_dir), ent_device != NULL) {
        /* Find the iio directory with the sensor definition */
        if (ent_device->d_type != DT_LNK)
            continue;
        char path_device[IIO_MAX_DEVICE_NAME_LENGTH];
        strcpy(path_device, IIO_DIR);
        strcat(path_device, ent_device->d_name);

        char dev_name[IIO_MAX_NAME_LENGTH + 1];
        if (cros_ec_sysfs_read_attr(path_device, "name", dev_name))
            continue;

        for (int i = CROS_EC_ACCEL; i < CROS_EC_RING; ++i) {
            /* We assume only one sensor hub per device.
             * Otherwise we need to look at the symlink and connect the 2:
             * iio:device0 ->
             *  ../../../devices/7000c400.i2c/i2c-1/1-001e/cros-ec-dev.0/
             *  cros-ec-accel.0/iio:device0
             * and
             * ...
             * iio:device1 ->
             *  ../../../devices/7000c400.i2c/i2c-1/1-001e/cros-ec-dev.0/
             *  cros-ec-ring.0/iio:device1
             */
            if (!strcmp(cros_ec_sensor_names[i], dev_name)) {
                /*
                 * First check if the device belongs to the lid.
                 * (base is keyboard)
                 */
                char loc[IIO_MAX_NAME_LENGTH + 1];
                if (cros_ec_sysfs_read_attr(path_device, "location", loc))
                    continue;
                if (strcmp(cros_ec_location, loc))
                    continue;

                char dev_id[40];
                if (cros_ec_sysfs_read_attr(path_device, "id", dev_id))
                    continue;
                int sensor_id = atoi(dev_id);

                if (Stotal_max_sensor_handle_ <= sensor_id) {
                    Stotal_max_sensor_handle_ = sensor_id + 1;
                    Ssensor_info_ = (cros_ec_sensor_info*)realloc(Ssensor_info_,
                            Stotal_max_sensor_handle_ *
                            sizeof(cros_ec_sensor_info));
                    if (Ssensor_info_ == NULL)
                        return -ENOMEM;
                    memset(&Ssensor_info_[sensor_id], 0,
                           sizeof(cros_ec_sensor_info));
                }

                char dev_scale[40];
                if (cros_ec_sysfs_read_attr(path_device, "scale", dev_scale)) {
                    ALOGE("Unable to read scale\n");
                    return 0;
                }
                double scale = atof(dev_scale);

                Ssensor_info_[sensor_id].device_name =
                          strdup(ent_device->d_name);

                sensor_t *sensor_data;
                sensor_data = &Ssensor_info_[sensor_id].sensor_data;
                memcpy(sensor_data, &sSensorListTemplate[i], sizeof(sensor_t));
                sensor_data->handle = sensor_id;

                if (sensor_data->type == SENSOR_TYPE_MAGNETIC_FIELD)
                    /* iio units are in Gauss, not micro Telsa */
                    scale *= 100;
                if (sensor_data->type == SENSOR_TYPE_PROXIMITY) {
                    /*
                     * Proximity does not detect anything beyond 3m.
                     */
                    sensor_data->resolution = 1;
                    sensor_data->maxRange = 300;
                } else {
                    sensor_data->resolution = scale;
                    sensor_data->maxRange = scale * (1 << 15);
                }

                ALOGD("new dev '%s' handle: %d\n",
                      Ssensor_info_[sensor_id].device_name, sensor_id);
                break;
            }
        }

        if (!strcmp(cros_ec_sensor_names[CROS_EC_RING], dev_name)) {
            *ring_device_name = strdup(ent_device->d_name);
        }

        char trigger_name[80];
        strcpy(trigger_name, cros_ec_sensor_names[CROS_EC_RING]);
        strcat(trigger_name, "-trigger");
        if (!strncmp(trigger_name, dev_name, strlen(trigger_name))) {
            *ring_trigger_name = strdup(dev_name);
            ALOGD("new trigger '%s' \n", *ring_trigger_name);
            continue;
        }
    }

    if (*ring_device_name == NULL || *ring_trigger_name == NULL)
        return -ENODEV;

    return Stotal_max_sensor_handle_ ? Stotal_max_sensor_handle_ : -ENODEV;
}

static struct hw_module_methods_t cros_ec_sensors_methods = {
    open: cros_ec_open_sensors,
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    common: {
      tag: HARDWARE_MODULE_TAG,
      version_major: 1,
      version_minor: 0,
      id: SENSORS_HARDWARE_MODULE_ID,
      name: "CrosEC sensor hub module",
      author: "Google",
      methods: &cros_ec_sensors_methods,
      dso: NULL,
      reserved: { 0 },
    },
    get_sensors_list: cros_ec_get_sensors_list,
    set_operation_mode: NULL,
};

/*****************************************************************************/

/*
 * cros_ec_sensors_poll_context_t:
 *
 * Responsible for implementing the pool functions.
 * We are currently polling on 2 files:
 * - the IIO ring buffer (via CrosECSensor object)
 * - a pipe to sleep on. a call to activate() will wake up
 *   the context poll() is running in.
 *
 * This code could accomodate more than one ring buffer.
 * If we implement wake up/non wake up sensors, we would lister to
 * iio buffer woken up by sysfs triggers.
 */
struct cros_ec_sensors_poll_context_t {
    sensors_poll_device_1_t device; // must be first

    cros_ec_sensors_poll_context_t(
         const char *ring_device_name,
         const char *ring_trigger_name);
    ~cros_ec_sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);
    int batch(int handle, int flags, int64_t period_ns, int64_t timeout);
    int flush(int handle);

    private:
    enum {
        crosEcRingFd           = 0,
        crosEcWakeFd,
        numFds,
    };

    static const char WAKE_MESSAGE = 'W';
    struct pollfd mPollFds[numFds];
    int mWritePipeFd;
    CrosECSensor *mSensor;

};

cros_ec_sensors_poll_context_t::cros_ec_sensors_poll_context_t(
         const char *ring_device_name,
         const char *ring_trigger_name)
{
    /*
     * One more time, assume only one sensor hub in the system.
     * Find the iio:deviceX with name "cros_ec_ring"
     * Open /dev/iio:deviceX, enable buffer.
     */
    mSensor = new CrosECSensor(Ssensor_info_,
        ring_device_name, ring_trigger_name);

    mPollFds[crosEcRingFd].fd = mSensor->getFd();
    mPollFds[crosEcRingFd].events = POLLIN;
    mPollFds[crosEcRingFd].revents = 0;

    int wakeFds[2];
    int result = pipe(wakeFds);
    ALOGE_IF(result < 0, "error creating wake pipe (%s)", strerror(errno));
    fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
    fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
    mWritePipeFd = wakeFds[1];

    mPollFds[crosEcWakeFd].fd = wakeFds[0];
    mPollFds[crosEcWakeFd].events = POLLIN;
    mPollFds[crosEcWakeFd].revents = 0;
}

cros_ec_sensors_poll_context_t::~cros_ec_sensors_poll_context_t() {
    delete mSensor;
    close(mPollFds[crosEcWakeFd].fd);
    close(mWritePipeFd);
}

int cros_ec_sensors_poll_context_t::activate(int handle, int enabled) {
    int err = mSensor->activate(handle, enabled);

    if (enabled && !err) {
        const char wakeMessage(WAKE_MESSAGE);
        int result = write(mWritePipeFd, &wakeMessage, 1);
        ALOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
    }
    return err;
}

int cros_ec_sensors_poll_context_t::setDelay(int /* handle */,
                                             int64_t /* ns */) {
    /* No supported */
    return 0;
}

int cros_ec_sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;
    do {
        // see if we have some leftover from the last poll()
        if (mPollFds[crosEcRingFd].revents & POLLIN) {
            int nb = mSensor->readEvents(data, count);
            if (nb < count) {
                // no more data for this sensor
                mPollFds[crosEcRingFd].revents = 0;
            }
            count -= nb;
            nbEvents += nb;
            data += nb;
        }

        if (count) {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait if we don't have
            // anything to return
            do {
                TEMP_FAILURE_RETRY(n = poll(mPollFds, numFds,
                                            nbEvents ? 0 : -1));
            } while (n < 0 && errno == EINTR);
            if (n < 0) {
                ALOGE("poll() failed (%s)", strerror(errno));
                return -errno;
            }
            if (mPollFds[crosEcWakeFd].revents & POLLIN) {
                char msg(WAKE_MESSAGE);
                int result = read(mPollFds[crosEcWakeFd].fd, &msg, 1);
                ALOGE_IF(result < 0,
                         "error reading from wake pipe (%s)", strerror(errno));
                ALOGE_IF(msg != WAKE_MESSAGE,
                         "unknown message on wake queue (0x%02x)", int(msg));
                mPollFds[crosEcWakeFd].revents = 0;
            }
        }
        // if we have events and space, go read them
    } while (n && count);
    return nbEvents;
}

int cros_ec_sensors_poll_context_t::batch(int handle, int /* flags */,
        int64_t sampling_period_ns,
        int64_t max_report_latency_ns)
{
    return mSensor->batch(handle, sampling_period_ns,
                          max_report_latency_ns);
}

int cros_ec_sensors_poll_context_t::flush(int handle)
{
    return mSensor->flush(handle);
}


/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    cros_ec_sensors_poll_context_t *ctx = reinterpret_cast<cros_ec_sensors_poll_context_t *>(dev);
    if (ctx) {
        delete ctx;
    }
    if (Stotal_max_sensor_handle_ != 0) {
        free(Ssensor_info_);
        Stotal_max_sensor_handle_ = 0;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    cros_ec_sensors_poll_context_t *ctx = reinterpret_cast<cros_ec_sensors_poll_context_t *>(dev);
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    cros_ec_sensors_poll_context_t *ctx = reinterpret_cast<cros_ec_sensors_poll_context_t *>(dev);
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    cros_ec_sensors_poll_context_t *ctx = reinterpret_cast<cros_ec_sensors_poll_context_t *>(dev);
    return ctx->pollEvents(data, count);
}

static int poll__batch(struct sensors_poll_device_1 *dev,
        int handle, int flags, int64_t period_ns, int64_t timeout)
{
    cros_ec_sensors_poll_context_t *ctx = (cros_ec_sensors_poll_context_t *)dev;
    return ctx->batch(handle, flags, period_ns, timeout);
}

static int poll__flush(struct sensors_poll_device_1 *dev,
        int handle)
{
    cros_ec_sensors_poll_context_t *ctx = (cros_ec_sensors_poll_context_t *)dev;
    return ctx->flush(handle);
}

/*****************************************************************************/

/*
 * cros_ec_open_sensors: open entry point.
 *
 * Call by sensor service via helper function:  sensors_open()
 *
 * Create a device the service will use for event polling.
 * Assume one open/one close.
 *
 * Later, sensorservice will use device with an handle to access
 * a particular sensor.
 */
static int cros_ec_open_sensors(
        const struct hw_module_t* module, const char*,
        struct hw_device_t** device)
{
    char *ring_device_name, *ring_trigger_name;
    int err;
    err = cros_ec_get_sensors_names(&ring_device_name, &ring_trigger_name);
    if (err < 0)
        return err;

    cros_ec_sensors_poll_context_t *dev = new cros_ec_sensors_poll_context_t(
            ring_device_name, ring_trigger_name);

    memset(&dev->device, 0, sizeof(sensors_poll_device_1_t));

    dev->device.common.tag      = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = SENSORS_DEVICE_API_VERSION_1_3;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;

    // Batch processing
    dev->device.batch           = poll__batch;
    dev->device.flush           = poll__flush;

    *device = &dev->device.common;

    return 0;
}

