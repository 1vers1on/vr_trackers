#include "hardware/usb.h"
#include "zephyr/usb/class/hid.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(usb, LOG_LEVEL_DBG);

static const uint8_t hid_report_descriptor[] = {
    HID_USAGE_PAGE(0xFF), // Vendor-defined usage page
    HID_USAGE(0x01),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

        // trackers 0-3
        HID_REPORT_ID(0x01),
        HID_USAGE(0x01),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX8(255),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(18), // 16 bytes quaternion + 1 byte device id + battery level
        HID_INPUT(0x02),

        // trackers 4-7
        HID_REPORT_ID(0x02),
        HID_USAGE(0x01),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX8(255),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(18), // 16 bytes quaternion + 1 byte device id + battery level
        HID_INPUT(0x02),

        // magnetometer calibration data
        HID_REPORT_ID(0x03),
        HID_USAGE(0x02),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX8(255),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(13), // 12 bytes xyz + 1 byte device id
        HID_INPUT(0x02),

    HID_END_COLLECTION,
};

static const struct device *hid_dev = DEVICE_DT_GET(DT_NODELABEL(usb_hid_0));

// TODO: make this not use deprecated APIs
int init_usb(void) {
    LOG_INF("Initializing USB HID device");
    if (!device_is_ready(hid_dev)) {
        LOG_ERR("USB HID device not ready");
        return -ENODEV;
    }

    usb_hid_register_device(hid_dev,
                                  hid_report_descriptor,
                                  sizeof(hid_report_descriptor),
                                  NULL);
    
    usb_hid_init(hid_dev);
    int ret = usb_enable(NULL);
    if (ret) {
        LOG_ERR("Failed to enable USB");
        return ret;
    }

    LOG_INF("USB initialized");

    return 0;
}
