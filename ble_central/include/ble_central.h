#ifndef BLE_H_
#define BLE_H_

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gap.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <console/console.h>
#include <errno.h>
#include <kernel.h>
#include <stddef.h>
#include <string.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <version.h>
#include <zephyr.h>
#include <zephyr/types.h>

/**
 * @brief UUID do bluetooth UART.
 */
#define BT_UART_UUID_SVC_VAL 0x0001

/**
 * @brief UUID do serviço UART.
 */
#define BT_UART_SVC_UUID BT_UUID_DECLARE_16(BT_UART_UUID_SVC_VAL)

/**
 * @brief UUID da característica da notificação da UART.
 */
#define BT_UART_NOTIFY_CHAR_UUID_VAL 0x0002

/**
 * @brief UUID da caracterísitica da notificação da UART.
 */
#define BT_UART_NOTIFY_CHAR_UUID BT_UUID_DECLARE_16(BT_UART_NOTIFY_CHAR_UUID_VAL)

/**
 * @brief UUID da característica da escrita da UART.
 */
#define BT_UART_WRITE_CHAR_UUID_VAL 0x0003

/**
 * @brief UUID da característica da escrita da UART.
 */
#define BT_UART_WRITE_CHAR_UUID BT_UUID_DECLARE_16(BT_UART_WRITE_CHAR_UUID_VAL)


/**
 * @brief Inicia o escaneamento do bluetooth.
 * @return 0 para sucesso, inteiro para falha.
 */
int start_central_scan();

/**
 * @brief Informa o status de conexão.
 * @return True se conectado, false caso contrário.
 */
bool check_connection();

/**
 * @brief Informa se a task pode pegar a próxima informação. Importante, pois o Renode trava esperando o texto, demorando a resposta.
 * @return True se já respondeu, false caso contrário.
 */
bool check_task_flag();

/**
 * @brief Envia dados para o periférico.
 * @param[in] buffer Buffer contendo os dados a serem enviados.
 * @param buffer_size Tamanho do buffer.
 */
void send_data(char buffer[], int buffer_size);

/**
 * @brief Inicia o bluetooth central.
 * @return 0 para sucesso, inteiro negativo para falha
 */
int ble_central_init();

#endif