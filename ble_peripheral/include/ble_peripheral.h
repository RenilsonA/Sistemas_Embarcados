#ifndef BLE_PERIPHERAL_H_
#define BLE_PERIPHERAL_H_

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <console/console.h>
#include <ctype.h>
#include <errno.h>
#include <kernel.h>
#include <stddef.h>
#include <string.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <version.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "stdint.h"
#include "stdlib.h"

/**
 * @brief Valor do UUID do serviço BT UART.
 *
 */
#define BT_UART_UUID_SVC_VAL 0x0001

/**
 * @brief UUID do serviço BT UART.
 *
 */
#define BT_UART_SVC_UUID BT_UUID_DECLARE_16(BT_UART_UUID_SVC_VAL)

/**
 * @brief Valor do UUID da característica de notificação do BT UART.
 *
 */
#define BT_UART_NOTIFY_CHAR_UUID_VAL 0x0002

/**
 * @brief UUID da caracterísitica de notificação do BT UART.
 *
 */
#define BT_UART_NOTIFY_CHAR_UUID BT_UUID_DECLARE_16(BT_UART_NOTIFY_CHAR_UUID_VAL)

/**
 * @brief Valor do UUID da característica de escrita do BT UART.
 *
 */
#define BT_UART_WRITE_CHAR_UUID_VAL 0x0003

/**
 * @brief UUID da característica de escrita do BT UART.
 *
 */
#define BT_UART_WRITE_CHAR_UUID BT_UUID_DECLARE_16(BT_UART_WRITE_CHAR_UUID_VAL)

/**
 * @brief Inicia o periférico BLE.
 * 
 * @return 0 para sucesso, inteiro negativo para falha.
 */
int ble_peripheral();

#endif