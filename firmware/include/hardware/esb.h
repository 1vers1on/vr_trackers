#ifndef HARDWARE_ESB_H
#define HARDWARE_ESB_H

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include "protocol/data.h"

typedef struct {
    struct k_work work;
    uint8_t tracker_id;
    tracker_data_t imu;
} esb_work_item_t;

typedef void (*esb_data_callback_t)(tracker_data_t *data, uint8_t tracker_id);

int init_esb(bool is_receiver);
int start_esb(void);
int send_imu_data(tracker_data_t *data, uint8_t tracker_id);

void esb_register_data_callback(esb_data_callback_t callback);

#endif // HARDWARE_ESB_H
