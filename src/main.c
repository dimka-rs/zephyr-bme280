/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS (1000)
#define LED_PIN (4)
#define SCL_PIN (5)
#define SDA_PIN (12)

#define BME280 DT_INST(0, bosch_bme280)
#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif


void main(void)
{
    const struct device *gpio0;
    const struct device *bme280;
    bool led_is_on = true;
    int ret;

    printk("Build date: " __DATE__ " " __TIME__"\n");

    gpio0 = device_get_binding("GPIO_0");
    if (gpio0 == NULL)
    {
        return;
    } else {
        ret = gpio_pin_configure(gpio0, LED_PIN, GPIO_OUTPUT);
        if (ret < 0)
        {
            return;
        }
    }

    bme280 = device_get_binding(BME280_LABEL);
    if (bme280 == NULL) {
        printk("%s not found\n", BME280_LABEL);
    } else {
        printk("%s found\n", BME280_LABEL);
    }

    while (1) {
        struct sensor_value temp, press, humidity;

        if (bme280 != NULL)
        {
            sensor_sample_fetch(bme280);
            sensor_channel_get(bme280, SENSOR_CHAN_AMBIENT_TEMP, &temp);
            sensor_channel_get(bme280, SENSOR_CHAN_PRESS, &press);
            sensor_channel_get(bme280, SENSOR_CHAN_HUMIDITY, &humidity);

            printk("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d\n",
                temp.val1, temp.val2, press.val1, press.val2,
                humidity.val1, humidity.val2);
        }

        gpio_pin_set(gpio0, LED_PIN, (int)led_is_on);
        led_is_on = !led_is_on;
        k_sleep(K_MSEC(SLEEP_TIME_MS));
    }
}
