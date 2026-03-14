#include "hardware/usb.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_hid.h>

LOG_MODULE_REGISTER(usb, LOG_LEVEL_DBG);

static const char *const blocklist[] = {
	"dfu_dfu",
	NULL,
};


USBD_DEVICE_DEFINE(usbd,
		   DEVICE_DT_GET(DT_NODELABEL(usbd)),
		   0x1209, 0x0001);

USBD_DESC_LANG_DEFINE(usb_lang);
USBD_DESC_MANUFACTURER_DEFINE(usb_mfr, "Ellie");
USBD_DESC_PRODUCT_DEFINE(usb_product, "VR Tracker");

USBD_DESC_CONFIG_DEFINE(fs_cfg_desc, "FS Configuration");
USBD_DESC_CONFIG_DEFINE(hs_cfg_desc, "HS Configuration");

USBD_CONFIGURATION_DEFINE(fs_config, USB_SCD_SELF_POWERED, 250, &fs_cfg_desc);
USBD_CONFIGURATION_DEFINE(hs_config, USB_SCD_SELF_POWERED, 250, &hs_cfg_desc);

static const uint8_t hid_report_descriptor[] = {
    HID_USAGE_PAGE(0xFF), // Vendor-defined usage page
    HID_USAGE(0x01),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

        // trackers 0-3
        HID_REPORT_ID(0x01),
        HID_USAGE(0x01),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX16(255, 0),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(18), // 16 bytes quaternion + 1 byte device id + battery level
        HID_INPUT(0x02),

        // trackers 4-7
        HID_REPORT_ID(0x02),
        HID_USAGE(0x01),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX16(255, 0),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(18), // 16 bytes quaternion + 1 byte device id + battery level
        HID_INPUT(0x02),

        // magnetometer calibration data
        HID_REPORT_ID(0x03),
        HID_USAGE(0x02),
        HID_LOGICAL_MIN8(0),
        HID_LOGICAL_MAX16(255, 0),
        HID_REPORT_SIZE(8),
        HID_REPORT_COUNT(13), // 12 bytes xyz + 1 byte device id
        HID_INPUT(0x02),

    HID_END_COLLECTION,
};

static const struct device *hid_dev = DEVICE_DT_GET(DT_NODELABEL(hid_dev_0));

static bool device_ready;

static void device_iface_ready(const struct device *dev, const bool ready) {
	LOG_INF("HID device %s interface is %s",
		dev->name, ready ? "ready" : "not ready");
	device_ready = ready;
}

static int device_get_report(const struct device *dev,
			 const uint8_t type, const uint8_t id, const uint16_t len,
			 uint8_t *const buf) {
	LOG_WRN("Get Report not implemented, Type %u ID %u", type, id);

	return 0;
}

struct hid_device_ops device_ops = {
	.iface_ready = device_iface_ready,
	.get_report = device_get_report,
};

int init_usb(void) {
    LOG_INF("Initializing USB HID device");
    if (!device_is_ready(hid_dev)) {
        LOG_ERR("USB HID device not ready");
        return -ENODEV;
    }

    int ret;

    ret = usbd_add_descriptor(&usbd, &usb_lang);
    if (ret) {
        LOG_ERR("Failed to add language descriptor");
        return ret;
    }

    ret = usbd_add_descriptor(&usbd, &usb_mfr);
    if (ret) {
        LOG_ERR("Failed to add manufacturer descriptor");
        return ret;
    }

    ret = usbd_add_descriptor(&usbd, &usb_product);
    if (ret) {
        LOG_ERR("Failed to add product descriptor");
        return ret;
    }

    ret = usbd_add_configuration(&usbd, USBD_SPEED_HS,
					     &hs_config);
    if (ret) {
        LOG_ERR("Failed to add HS configuration");
        return ret;
    }

    ret = usbd_register_all_classes(&usbd, USBD_SPEED_HS, 1,
						blocklist);
    if (ret) {
        LOG_ERR("Failed to register classes for HS");
        return ret;
    }

    ret = usbd_add_configuration(&usbd, USBD_SPEED_FS,
				     &fs_config);
    if (ret) {
        LOG_ERR("Failed to add FS configuration");
        return ret;
    }

    ret = usbd_register_all_classes(&usbd, USBD_SPEED_FS, 1,
                        blocklist);
    if (ret) {
        LOG_ERR("Failed to register classes for FS");
        return ret;
    }

    ret = hid_device_register(hid_dev,
				  hid_report_descriptor, sizeof(hid_report_descriptor),
				  &device_ops);
    if (ret) {
        LOG_ERR("Failed to register HID device");
        return ret;
    }

    ret = usbd_init(&usbd);
    if (ret) {
        LOG_ERR("Failed to initialize USB");
        return ret;
    }

    ret = usbd_enable(&usbd);
    if (ret) {
        LOG_ERR("Failed to enable USB");
        return ret;
    }

    LOG_INF("USB initialized");

    return 0;
}
