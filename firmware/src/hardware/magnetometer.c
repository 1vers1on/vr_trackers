#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "hardware/magnetometer.h"

LOG_MODULE_REGISTER(magnetometer, LOG_LEVEL_DBG);

static const struct device *const mag_dev = DEVICE_DT_GET(DT_NODELABEL(bmm350));

static struct mag_calibration current_calibration = {
    .offset = {0.0f, 0.0f, 0.0f},
    .scale = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    }
};

int init_magnetometer(void) {
    LOG_INF("Initializing magnetometer");
    if (!device_is_ready(mag_dev)) {
        LOG_ERR("BMM350 device not ready");
        return -ENODEV;
    }
    LOG_INF("Magnetometer initialized successfully");
    return 0;
}

void magnetometer_set_calibration(const mag_calibration_t *cal) {
    if (cal) {
        current_calibration = *cal;
        LOG_INF("New calibration applied");
    }
}

int magnetometer_read_raw(mag_data_t *output) {
    struct sensor_value raw_val[3];
    int ret;

    if (output == NULL) {
        return -EINVAL;
    }

    ret = sensor_sample_fetch(mag_dev);
    if (ret) {
        LOG_ERR("Sample fetch failed: %d", ret);
        return ret;
    }

    ret = sensor_channel_get(mag_dev, SENSOR_CHAN_MAGN_XYZ, raw_val);
    if (ret) {
        LOG_ERR("Channel get failed: %d", ret);
        return ret;
    }

    output->x = (float)sensor_value_to_double(&raw_val[0]);
    output->y = (float)sensor_value_to_double(&raw_val[1]);
    output->z = (float)sensor_value_to_double(&raw_val[2]);

    return 0;
}

int magnetometer_read(mag_data_t *output) {
    struct sensor_value raw_val[3];
    int ret;

    ret = sensor_sample_fetch(mag_dev);
    if (ret) {
        LOG_ERR("Sample fetch failed: %d", ret);
        return ret;
    }

    ret = sensor_channel_get(mag_dev, SENSOR_CHAN_MAGN_XYZ, raw_val);
    if (ret) return ret;

    float raw[3];
    for (int i = 0; i < 3; i++) {
        raw[i] = (float)sensor_value_to_double(&raw_val[i]);
    }

    float centered[3];
    centered[0] = raw[0] - current_calibration.offset[0];
    centered[1] = raw[1] - current_calibration.offset[1];
    centered[2] = raw[2] - current_calibration.offset[2];

    output->x = (current_calibration.scale[0][0] * centered[0]) + (current_calibration.scale[0][1] * centered[1]) + (current_calibration.scale[0][2] * centered[2]);
    output->y = (current_calibration.scale[1][0] * centered[0]) + (current_calibration.scale[1][1] * centered[1]) + (current_calibration.scale[1][2] * centered[2]);
    output->z = (current_calibration.scale[2][0] * centered[0]) + (current_calibration.scale[2][1] * centered[1]) + (current_calibration.scale[2][2] * centered[2]);

    return 0;
}
