#ifndef LED_H
#define LED_H

/* Queue to talk to LED thread */
extern struct k_queue queue_led;

/* Data to be passed by queue */
struct queue_led_item_t
{
    uint32_t reserved;
    uint32_t led_state;
};

#endif /* LED_H */
