#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#include <zephyr/drivers/sensor.h>

typedef struct mag_calibration {
    float offset[3];
    float scale[3][3];
} mag_calibration_t;

typedef struct mag_data {
    float x;
    float y;
    float z;
} mag_data_t;

int init_magnetometer(void);

int magnetometer_read(mag_data_t *output);

int magnetometer_read_raw(mag_data_t *output);

void magnetometer_set_calibration(const mag_calibration_t *cal);

#endif /* MAGNETOMETER_H */
