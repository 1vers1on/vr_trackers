#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

#include "hardware/imu.h"

LOG_MODULE_REGISTER(imu, LOG_LEVEL_DBG);

static const struct device *const imu_dev = DEVICE_DT_GET(DT_NODELABEL(lsm6dsv));

static gyro_calibration_data_t gyro_calibration_data = {
    .bias = {0.0, 0.0, 0.0}
};

static accel_calibration_data_t accel_calibration_data = {
    .bias = {0.0, 0.0, 0.0},
    .scale = {1.0, 1.0, 1.0}
};

int init_imu(void) {
    LOG_INF("Initializing IMU");
    if (!device_is_ready(imu_dev)) {
        LOG_ERR("LSM6DSV device not ready");
        return -ENODEV;
    }
    LOG_INF("IMU initialized successfully");
    // TODO: Figure out how to set the data rate
    return 0;
}

int set_gyroscope_calibration(const gyro_calibration_data_t *calibration_data) {
    if (calibration_data == NULL) {
        return -EINVAL;
    }
    gyro_calibration_data = *calibration_data;
    return 0;
}

int set_accelerometer_calibration(const accel_calibration_data_t *calibration_data) {
    if (calibration_data == NULL) {
        return -EINVAL;
    }
    accel_calibration_data = *calibration_data;
    return 0;
}

int imu_read_accelerometer_raw(imu_data_t *output) {
    struct sensor_value accel[3];
    int ret = sensor_sample_fetch(imu_dev);
    if (ret < 0) {
        LOG_ERR("Failed to fetch IMU sample");
        return ret;
    }
    ret = sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
    if (ret < 0) {
        LOG_ERR("Failed to get accelerometer data");
        return ret;
    }
    output->x = sensor_value_to_double(&accel[0]);
    output->y = sensor_value_to_double(&accel[1]);
    output->z = sensor_value_to_double(&accel[2]);
    return 0;
}

int imu_read_gyroscope_raw(imu_data_t *output) {
    struct sensor_value gyro[3];
    int ret = sensor_sample_fetch(imu_dev);
    if (ret < 0) {
        LOG_ERR("Failed to fetch IMU sample");
        return ret;
    }
    ret = sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_XYZ, gyro);
    if (ret < 0) {
        LOG_ERR("Failed to get gyroscope data");
        return ret;
    }
    output->x = sensor_value_to_double(&gyro[0]);
    output->y = sensor_value_to_double(&gyro[1]);
    output->z = sensor_value_to_double(&gyro[2]);
    return 0;
}

int calibrate_gyroscope(gyro_calibration_data_t *calibration_data) {
    struct sensor_value gyro[3];
    double sum_x = 0, sum_y = 0, sum_z = 0;
    int samples = 500;
    
    LOG_INF("Starting gyroscope calibration with %d samples", samples);
    for (int i = 0; i < samples; i++) {
        int ret = sensor_sample_fetch(imu_dev);
        if (ret < 0) {
            LOG_ERR("Failed to fetch IMU sample during calibration");
            return ret;
        }
        ret = sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_XYZ, gyro);
        if (ret < 0) {
            LOG_ERR("Failed to get gyroscope data during calibration");
            return ret;
        }
        sum_x += sensor_value_to_double(&gyro[0]);
        sum_y += sensor_value_to_double(&gyro[1]);
        sum_z += sensor_value_to_double(&gyro[2]);

        k_msleep(10);
    }

    calibration_data->bias[0] = sum_x / samples;
    calibration_data->bias[1] = sum_y / samples;
    calibration_data->bias[2] = sum_z / samples;

    LOG_INF("Gyroscope calibration completed: bias_x=%.2f, bias_y=%.2f, bias_z=%.2f",
            calibration_data->bias[0], calibration_data->bias[1], calibration_data->bias[2]);

    return 0;
}
