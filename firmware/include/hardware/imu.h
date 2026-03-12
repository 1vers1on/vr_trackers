#ifndef IMU_H
#define IMU_H

typedef struct {
    float x;
    float y;
    float z;
} imu_data_t;

typedef struct {
    double bias[3];
} gyro_calibration_data_t;

typedef struct {
    double bias[3];
    double scale[3];
} accel_calibration_data_t;

int init_imu(void);

int imu_read_accelerometer_raw(imu_data_t *output);
int imu_read_gyroscope_raw(imu_data_t *output);

int set_gyroscope_calibration(const gyro_calibration_data_t *calibration_data);
int set_accelerometer_calibration(const accel_calibration_data_t *calibration_data);

int calibrate_gyroscope(gyro_calibration_data_t *calibration_data);

#endif /* IMU_H */