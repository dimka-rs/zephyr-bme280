#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include <drivers/gpio.h>
#include <led.h>

#define STACKSIZE 1024
#define PRIORITY 7

#define LED_PIN (4)

K_QUEUE_DEFINE(queue_led);

void led_main(void *dummy1, void *dummy2, void *dummy3)
{
    ARG_UNUSED(dummy1);
    ARG_UNUSED(dummy2);
    ARG_UNUSED(dummy3);
    const struct device *gpio0;
    int ret;

    /* Configure LED */
    gpio0 = device_get_binding("GPIO_0");
    if (gpio0 == NULL)
    {
        printk("Failed to get GPIO_0\n");
        return;
    } else {
        ret = gpio_pin_configure(gpio0, LED_PIN, GPIO_OUTPUT);
        if (ret < 0)
        {
            printk("Failed to configure LED pin\n");
            return;
        }
    }

    while(1)
    {
        struct queue_led_item_t *queue_led_item = k_queue_get(&queue_led, K_FOREVER);
        gpio_pin_set(gpio0, LED_PIN, queue_led_item->led_state);
    }
}

K_THREAD_DEFINE(thread_led, STACKSIZE, led_main, NULL, NULL, NULL, PRIORITY, 0, 0);
