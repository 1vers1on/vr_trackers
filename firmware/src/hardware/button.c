#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "hardware/button.h"
#include "config.h"

LOG_MODULE_REGISTER(button, LOG_LEVEL_DBG);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);
static struct gpio_callback button_cb_data;

static atomic_t press_count = ATOMIC_INIT(0);
struct k_work_delayable click_work;

static click_callback_t callbacks[CLICK_COUNT] = {0};
K_MSGQ_DEFINE(click_msgq, sizeof(click_type_t), 10, 4);

void click_work_handler(struct k_work *work) {
    int counts = atomic_get(&press_count);
    atomic_set(&press_count, 0);
    
    click_type_t detected;

    if (counts == 1)      detected = SINGLE_CLICK;
    else if (counts == 2) detected = DOUBLE_CLICK;
    else if (counts >= 3) detected = TRIPLE_CLICK;
    else return;

    if (detected < CLICK_COUNT && callbacks[detected]) {
        callbacks[detected]();
    }
    
    k_msgq_put(&click_msgq, &detected, K_NO_WAIT);
}

void button_pressed_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    static int64_t last_time = 0;
    int64_t now = k_uptime_get();

    if (now - last_time < BUTTON_DEBOUNCE_MS) return;
    
    last_time = now;
    atomic_inc(&press_count);
    
    k_work_reschedule(&click_work, K_MSEC(300));
}

int init_button(void) {
    LOG_INF("Initializing button GPIO");
    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("Button GPIO device not ready");
        return -ENODEV;
    }
    int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure button GPIO pin");
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure button GPIO interrupt");
        return ret;
    }

    k_work_init_delayable(&click_work, click_work_handler);
    gpio_init_callback(&button_cb_data, button_pressed_isr, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret < 0) {
        LOG_ERR("Failed to add button GPIO callback");
        return ret;
    }

    LOG_INF("Button GPIO initialized successfully");
    return 0;
}

int is_button_pressed(void) {
    int ret = gpio_pin_get_dt(&button);
    if (ret < 0) {
        LOG_ERR("Failed to read button GPIO pin");
        return ret;
    }
    return ret;
}

void wait_for_click(click_type_t target) {
    click_type_t received;
    while (1) {
        k_msgq_get(&click_msgq, &received, K_FOREVER);
        if (received == target) return;
    }
}

void register_click_callback(click_type_t type, click_callback_t cb) {
    callbacks[type] = cb;
}
