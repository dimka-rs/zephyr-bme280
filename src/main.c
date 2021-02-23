/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <led.h>
#include <ble.h>

#define SLEEP_TIME_MS (10000)

#define BME280 DT_INST(0, bosch_bme280)
#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif


struct queue_led_item_t queue_led_item;

void main(void)
{
    const struct device *bme280;

    printk("Build date: " __DATE__ " " __TIME__"\n");

    /* Init env sensor */
    bme280 = device_get_binding(BME280_LABEL);
    if (bme280 == NULL) {
        printk("%s not found\n", BME280_LABEL);
    } else {
        printk("%s found\n", BME280_LABEL);
    }

    /* Main loop */
    while (1) {
        struct sensor_value temp, press, humidity;

        /* Indicate activity start */
        queue_led_item.led_state = 1;
        k_queue_append(&queue_led, (void *)&queue_led_item);

        uint64_t uptime = k_uptime_get();
        ble_update(uptime);

        if (bme280 != NULL)
        {
            sensor_sample_fetch(bme280);
            sensor_channel_get(bme280, SENSOR_CHAN_AMBIENT_TEMP, &temp);
            sensor_channel_get(bme280, SENSOR_CHAN_PRESS, &press);
            sensor_channel_get(bme280, SENSOR_CHAN_HUMIDITY, &humidity);

            printk("[%6lld.%03lld] T: %d.%02d C; P: %d.%02d kPa; H: %d.%02d %%\n",
                uptime/1000, uptime%1000, temp.val1, temp.val2/10000,
                press.val1, press.val2/10000, humidity.val1, humidity.val2/10000);
        }

        /* Indicate activity stop */
        queue_led_item.led_state = 0;
        k_queue_append(&queue_led, (void *)&queue_led_item);

        k_sleep(K_MSEC(SLEEP_TIME_MS));
    }
}
