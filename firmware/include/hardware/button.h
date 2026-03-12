#ifndef BUTTON_H
#define BUTTON_H

typedef enum {
    SINGLE_CLICK,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    CLICK_COUNT // used for array sizing, not an actual click type
} click_type_t;

typedef void (*click_callback_t)(void);

int init_button(void);
int is_button_pressed(void);

void wait_for_click(click_type_t type);

void register_click_callback(click_type_t type, click_callback_t cb);

#endif /* BUTTON_H */
