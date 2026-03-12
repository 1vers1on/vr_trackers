#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "hardware/magnetometer.h"
#include "hardware/imu.h"
#include "hardware/button.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static const struct device *const fuel_dev = DEVICE_DT_GET(DT_NODELABEL(max17048));
static const struct gpio_dt_spec charger_status = GPIO_DT_SPEC_GET(DT_NODELABEL(charger_stat), gpios);

static int init_fuel_gauge(void) {
    LOG_INF("Initializing fuel gauge");
    if (!device_is_ready(fuel_dev)) {
        LOG_ERR("MAX17048 device not ready");
        return -ENODEV;
    }
    LOG_INF("Fuel gauge initialized successfully");
    return 0;
}

static int init_charger_status(void) {
    LOG_INF("Initializing charger status GPIO");
    if (!gpio_is_ready_dt(&charger_status)) {
        LOG_ERR("Charger status GPIO device not ready");
        return -ENODEV;
    }
    int ret = gpio_pin_configure_dt(&charger_status, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure charger status GPIO pin");
        return ret;
    }
    LOG_INF("Charger status GPIO initialized successfully");
    return 0;
}

int main() {
    init_imu();
    init_magnetometer();
    init_fuel_gauge();
    init_button();
    init_charger_status();

    while (1) {
        LOG_INF("Hello, Zephyr!");
        k_sleep(K_SECONDS(1));
    }
}