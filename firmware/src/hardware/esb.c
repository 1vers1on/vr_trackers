#include "hardware/esb.h"
#include "protocol/data.h"
#include <stdint.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <esb.h>
#include "config.h"

LOG_MODULE_REGISTER(esb, LOG_LEVEL_DBG);

static bool receiver_mode = false;

static uint8_t global_seq_num = 0;
static uint8_t last_seq[8] = {0};

static esb_work_item_t work_pool[ESB_WORK_POOL_SIZE];
static uint8_t pool_idx = 0;

static esb_data_callback_t app_callback = NULL;


void esb_register_data_callback(esb_data_callback_t callback) {
    app_callback = callback;
}

static void esb_work_handler(struct k_work *work) {
    esb_work_item_t *item = CONTAINER_OF(work, esb_work_item_t, work);

    if (app_callback) {
        app_callback(&item->imu, item->tracker_id);
    }
}

static void check_and_update_seq(uint8_t tracker_id, uint8_t seq_num) {
    if (tracker_id >= 8) {
        LOG_WRN("Received packet with invalid tracker ID: %d", tracker_id);
        return;
    }

    uint8_t expected_seq = last_seq[tracker_id] + 1;
    if (seq_num != expected_seq) {
        LOG_WRN("Packet loss detected for Tracker %d! Expected seq %d but got %d", 
                tracker_id, expected_seq, seq_num);
    }
    last_seq[tracker_id] = seq_num;
}

void rx_esb_event_handler(struct esb_evt const *event) {
    if (event->evt_id == ESB_EVENT_RX_RECEIVED) {
        struct esb_payload rx_payload;
        
        while (esb_read_rx_payload(&rx_payload) == 0) {
            if (rx_payload.data[0] == PACKET_TYPE_IMU) {
                tracker_data_t *data = (tracker_data_t *)rx_payload.data;
                check_and_update_seq(rx_payload.pipe, data->seq_num);

                esb_work_item_t *item = &work_pool[pool_idx];
                pool_idx = (pool_idx + 1) % ESB_WORK_POOL_SIZE;

                k_work_init(&item->work, esb_work_handler);

                item->tracker_id = rx_payload.pipe;
                memcpy(&item->imu, data, sizeof(tracker_data_t));

                k_work_submit(&item->work);
            } else {
                LOG_WRN("Received unknown packet type 0x%02X on pipe %d", rx_payload.data[0], rx_payload.pipe);
            }
        }
    }
}

void tx_esb_event_handler(struct esb_evt const *event) {
    // TODO: do something idk
}

int init_esb(bool is_receiver) {
    LOG_INF("Initializing ESB");
    
    if (is_receiver) {
        receiver_mode = true;
        LOG_INF("Configuring ESB as receiver");
        struct esb_config config = ESB_DEFAULT_CONFIG;
        config.protocol = ESB_PROTOCOL_ESB_DPL;
        config.bitrate = ESB_BITRATE_2MBPS;
        config.mode = ESB_MODE_PRX;
        config.event_handler = rx_esb_event_handler;
        config.selective_auto_ack = true;

        int ret = esb_init(&config);
        if (ret < 0) {
            LOG_ERR("Failed to initialize ESB: %d", ret);
            return ret;
        }

        uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
        uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};

        uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8};

        esb_set_base_address_0(base_addr_0);
        esb_set_base_address_1(base_addr_1);
        esb_set_prefixes(addr_prefix, 8);
    } else {
        LOG_INF("Configuring ESB as transmitter");
        struct esb_config config = ESB_DEFAULT_CONFIG;
        config.protocol = ESB_PROTOCOL_ESB_DPL;
        config.bitrate = ESB_BITRATE_2MBPS;
        config.mode = ESB_MODE_PTX;
        config.event_handler = tx_esb_event_handler;
        config.selective_auto_ack = true;

        int ret = esb_init(&config);
        if (ret < 0) {
            LOG_ERR("Failed to initialize ESB: %d", ret);
            return ret;
        }

        uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
        uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
        uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8};

        esb_set_base_address_0(base_addr_0);
        esb_set_base_address_1(base_addr_1);
        esb_set_prefixes(addr_prefix, 8);
    }

    return 0;
}

int start_esb(void) {
    if (receiver_mode) {
        LOG_INF("Starting ESB receiver");
        int ret = esb_start_rx();
        if (ret < 0) {
            LOG_ERR("Failed to start ESB receiver: %d", ret);
            return ret;
        }

    } else {
        LOG_INF("ESB transmitter ready");
        int ret = esb_start_tx();
        if (ret < 0) {
            LOG_ERR("Failed to start ESB transmitter: %d", ret);
            return ret;
        }
    }
    return 0;
}

int send_imu_data(tracker_data_t *data, uint8_t tracker_id) {
    if (receiver_mode) {
        LOG_WRN("Cannot send data over ESB in receiver mode");
        return -1;
    }

    data->seq_num = global_seq_num++;

    struct esb_payload tx_payload;
    tx_payload.length = sizeof(tracker_data_t);
    tx_payload.pipe = tracker_id; // use tracker ID as pipe number
    tx_payload.noack = false; // request ACK from receiver
    memcpy(tx_payload.data, data, sizeof(tracker_data_t));

    int ret = esb_write_payload(&tx_payload);
    if (ret < 0) {
        LOG_ERR("Failed to send ESB payload: %d", ret);
        return ret;
    }

    return 0;
}
