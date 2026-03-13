#ifndef PROTOCOL_DATA_H
#define PROTOCOL_DATA_H

#include <stdint.h>

typedef enum {
    PACKET_TYPE_IMU = 0x01,
} packet_type_t;

typedef struct __attribute__((packed)) {
    uint8_t packet_type; // will always be 0x01 for this struct
    uint8_t seq_num; // set by transmitting functions, used by receiver to detect lost packets
    float quat_w;
    float quat_x;
    float quat_y;
    float quat_z;
    float mag_x;
    float mag_y;
    float mag_z;
    uint8_t battery;
    uint8_t status1;
} tracker_data_t;

#endif // PROTOCOL_DATA_H
