#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <stdio.h> //snprintf

#define STACKSIZE 1024
#define PRIORITY 7


typedef enum {
    BLE_NOT_READY,
    BLE_READY,
} ble_state_t;

static volatile ble_state_t ble_state = BLE_NOT_READY;
uint8_t devname[25];

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
        ble_state = BLE_READY;
    }


    /* For connectable advertising you would use
     * bt_le_oob_get_local().  For non-connectable non-identity
     * advertising an non-resolvable private address is used;
     * there is no API to retrieve that.
     */
}

void ble_update(uint64_t uptime)
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

    if (ble_state == BLE_READY)
    {
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
}

void ble_main(void)
{
    /* Initialize the Bluetooth Subsystem */
    int ret = bt_enable(bt_ready);
    if (ret) {
        printk("Bluetooth init failed (err %d)\n", ret);
    }
}

K_THREAD_DEFINE(thread_ble, STACKSIZE, ble_main, NULL, NULL, NULL, PRIORITY, 0, 0);
