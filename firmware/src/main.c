#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <esb.h>

const struct device *const mag_dev = DEVICE_DT_GET(DT_NODELABEL(bmm350));
const struct device *const imu_dev = DEVICE_DT_GET(DT_NODELABEL(lsm6dsv));
const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

void init_magnetometer(void) {
    if (!device_is_ready(mag_dev)) {
        printk("BMM350 device not ready\n");
        return;
    }
}

void init_imu(void) {
    if (!device_is_ready(imu_dev)) {
        printk("LSM6DSV device not ready\n");
        return;
    }
}

void init_button(void) {
    if (!gpio_is_ready_dt(&button)) {
        return;
    }

    gpio_pin_configure_dt(&button, GPIO_INPUT);
}

int main() {
    init_magnetometer();
    init_imu();
    init_button();

    while (1) {
        printk("Hello, Zephyr!\n");
        k_sleep(K_SECONDS(1));
    }
}