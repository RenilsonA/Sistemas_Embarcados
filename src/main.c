/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/console/console.h>
#include "stdint.h"
#include "stdlib.h"
#include "string.h"


static void central_mtu();

static void central_connected();

static void central_disconnected();

static struct {
    uint16_t write_handle;
    struct bt_conn_cb conn_callbacks;
    struct bt_gatt_cb gatt_callbacks;
    struct bt_gatt_discover_params discover_params;
    struct bt_gatt_subscribe_params subscribe_params;
    struct bt_conn *default_conn;
    struct bt_uuid_16 uuid;
} self = {
    .conn_callbacks =
        {
            .connected = central_connected,
            .disconnected = central_disconnected,
        },
    .gatt_callbacks =
        {
            .att_mtu_updated = central_mtu,
        },
    .discover_params = {0},
    .subscribe_params = {0},
    .default_conn = NULL,
    .uuid = BT_UUID_INIT_16(0),
    .write_handle = 0,
};

void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
    char dev[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(addr, dev, sizeof(dev));
    printk("Device found: %s, RSSI %i.\n", dev, rssi);

    if (type == BT_GAP_ADV_TYPE_ADV_IND || type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
        
    }
}

void ble_scan(void) {
    int err = 0;

    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    err = bt_le_scan_start(&scan_param, device_found);
    if (err) {
        printk("Error scan: err %d.\n", err);
        return;
    }

    printk("Init scan.\n");
}

void central_connected(struct bt_conn *conn, uint8_t err){
    char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printk("Failed to connect to %s (%u)\n", addr, err);

		bt_conn_unref(self.default_conn);
		self.default_conn = NULL;

		start_scan();
		return;
	}

	if (conn != self.default_conn) {
		return;
	}
    printk("Central connected: %s.\n", addr);

	bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

void central_disconnected(struct bt_conn *conn, uint8_t reason){
    char addr[BT_ADDR_LE_STR_LEN];

	if (conn != self.default_conn) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Central disconnected: %s (reason 0x%02x)\n", addr, reason);

	bt_conn_unref(self.default_conn);
	self.default_conn = NULL;

	start_scan();
}

void central_mtu(){
    
}

void main(){
    int err = 0;

    bt_conn_cb_register(&self.conn_callbacks);
    bt_gatt_cb_register(&self.gatt_callbacks);

    err = bt_enable(ble_scan);
    if (err) {
        printk("BLE fail: err %d.\n", err);
        return err;
    }
    printk("BLE initialized;\n");
}