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
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <stdio.h> //snprintf

#define SLEEP_TIME_MS (10000)
#define LED_PIN (4)

#define BME280 DT_INST(0, bosch_bme280)
#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif

uint8_t devname[25];
static volatile uint8_t ble_adv_start = 0;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0xaa, 0xfe),
    BT_DATA_BYTES(BT_DATA_SVC_DATA16,
            0xaa, 0xfe, /* Eddystone UUID */
            0x10, /* Eddystone-URL frame type */
            0x00, /* Calibrated Tx power at 0m */
            0x00, /* URL Scheme Prefix http://www. */
            'z', 'e', 'p', 'h', 'y', 'r',
            'p', 'r', 'o', 'j', 'e', 'c', 't',
            0x08) /* .org */
};

static void bt_ready(int err)
{
    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;

    }
    else
    {
        ble_adv_start = 1;
    }


    /* For connectable advertising you would use
     * bt_le_oob_get_local().  For non-connectable non-identity
     * advertising an non-resolvable private address is used;
     * there is no API to retrieve that.
     */


}

void ble_update(void)
{
    char addr_s[BT_ADDR_LE_STR_LEN];
    bt_addr_le_t addr = {0};
    size_t count = 1;
    uint32_t uptime_s;
    int err;

    /* Set Scan Response data */
    static struct bt_data sd[] = {
        BT_DATA(BT_DATA_NAME_COMPLETE, &devname[0], 0)
    };

    uptime_s = k_uptime_get() / 1000;
    snprintf(devname, sizeof(devname), "%s:%d", "zephyr-bme280", uptime_s);
    sd[0].data_len = strlen(&devname[0]);

    /* Stop advertising */
    err = bt_le_adv_stop();
    if (err)
    {
        printk("Advertising failed to stop (err %d)\n", err);
    }
    /* Start advertising with new params */
    err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad),
            sd, ARRAY_SIZE(sd));
    if (err)
    {
        printk("Advertising failed to start (err %d)\n", err);
    }
    else
    {
        bt_id_get(&addr, &count);
        bt_addr_le_to_str(&addr, addr_s, sizeof(addr_s));
        printk("Beacon started, advertising as %s\n", addr_s);
    }
}

void main(void)
{
    const struct device *gpio0;
    const struct device *bme280;
    int ret;

    printk("Build date: " __DATE__ " " __TIME__"\n");

    /* Initialize the Bluetooth Subsystem */
    ret = bt_enable(bt_ready);
    if (ret) {
        printk("Bluetooth init failed (err %d)\n", ret);
    }

    /* Configure LED */
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

        gpio_pin_set(gpio0, LED_PIN, 1);
        if(ble_adv_start == 1)
        {
            ble_update();
        }

        uint64_t uptime = k_uptime_get();

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

        gpio_pin_set(gpio0, LED_PIN, 0);

        k_sleep(K_MSEC(SLEEP_TIME_MS));
    }
}
