#include "ble_peripheral.h"

static void mtu_callback(struct bt_conn *conn, uint16_t tx, uint16_t rx);
static void connected_callback(struct bt_conn *conn, uint8_t err);
static void disconnected_callback(struct bt_conn *conn, uint8_t reason);

static struct ble_t {
    uint16_t uart_write;                                /** Write da UART para o serviço */
    struct bt_conn_cb conn_callback;                    /** Bluetooth callback de conexões */
    struct bt_gatt_cb gatt_callback;                    /** Bluetooth callback GATT */
    struct bt_conn *default_conn;                       /** Estrutura de conexões do bluetooth*/
    struct bt_uuid_16 uuid_t;                           /** UUID para servico da UART */
    struct bt_gatt_discover_params discover_params;     /** Parâmetros GATT para descobrir dispositivos */
    struct bt_gatt_subscribe_params subscribe_params;   /** Parâmetros GATT para se inscrever nos dispositivos */
    struct bt_data ad[];                                /** Advertisement data to be broadcasted by the device. */
} self = {
    .conn_callback = {
        .connected    = connected_callback,
        .disconnected = disconnected_callback,
    },
    .gatt_callback = {
        .att_mtu_updated = mtu_callback,
    },
    .default_conn = NULL,
    .discover_params = {0},
    .uuid_t = BT_UUID_INIT_16(0),
    .subscribe_params = {0},
    .uart_write = 0,
    .ad = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UART_UUID_SVC_VAL), ),
    },
};

/**
 * @brief Callback para mensagem de modificação na Client Characteristic Configuration (CCC).
 * @param attr Atributo GATT.
 * @param value Novo valor CCC.
 */
static void notification_state(const struct bt_gatt_attr *attr, uint16_t value) {
    if(value == BT_GATT_CCC_NOTIFY){
        printk("Conectado e pronto para notirficar.");
    }
}

static void write_uart(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buffer, uint16_t lenght, uint16_t offset, uint8_t flags);

BT_GATT_SERVICE_DEFINE(ble_task, BT_GATT_PRIMARY_SERVICE(BT_UART_SVC_UUID), 
                       BT_GATT_CHARACTERISTIC(BT_UART_NOTIFY_CHAR_UUID, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
                       BT_GATT_CHARACTERISTIC(BT_UART_WRITE_CHAR_UUID, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL, write_uart, NULL), 
                       BT_GATT_CCC(notification_state, (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)));

/**
 * @brief Callback MTU.
 * @param conn Estrutura de conexão do bluetooth.
 * @param attr Aponta para atributo GATT.
 * @param buffer Buffer recebido.
 * @param lenght Tamanho do buffer recebido.
 * @param offset Deslocamento do valor atribuido.
 * @param flags Flags atriguidas com a operação.
*/
static void write_uart(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buffer, uint16_t lenght, uint16_t offset, uint8_t flags) {
    char string[lenght + 1];
    memcpy(string, buffer, lenght + 1);
    string[lenght] = '\0';
    printk("Dado recebido: %s\n", string);
    //Percorre a string e diminui uma constante para converter a letras maiúsculas.
    for (int i = 0; i < lenght; i++) {
        if ((string[i] >= 97) && ((string[i] <= 122))) {
            string[i] -= 32;
        }
    }
    //Colocar um fim na string (caso contrário, sempre mandará um espaço na memória até um \0 mais próximo, ocasionando erros).
    printk("Dado convertido: %s\n", string);
    int err = bt_gatt_notify(NULL, &ble_task.attrs[1], string, lenght + 1);
    if (err) {
        printk("Erro ao notificar. Valor: %d.\n", err);
    }
}

/**
* @brief Função para iniciar o anúncio do bluetooth periférico.
*/
static void init_advertising(){
    int err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, self.ad, 2, NULL, 0);
    if (err) {
        printk("Failed to start advertising. Error: %d.\n", err);
    } else {
        printk("Iniciando o anuncio.\n");
    }
}

/**
 * @brief Callback MTU.
 * @param conn Estrutura de conexão do bluetooth.
 * @param tx Bytes que podem ser enviados por pacote.
 * @param rx Bytes que podem ser recebidos por pacote.
*/
static void mtu_callback(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
    printk("MTU atualizado.\n");
}

/**
* @brief Callback de conexão.
* @param conn Estrutura de conexão do bluetooth.
* @param err The reason for the disconnection.
*/
static void connected_callback(struct bt_conn *conn, uint8_t err) {
    if (err) {
        printk("Falha para conectar. Erro: %u\n", err);
        return;
    }
    if (conn == self.default_conn) {
        self.default_conn = bt_conn_ref(conn);
        printk("Periférico conectado.\n");
    }
}

/**
* @brief Callback de desconexão.
* @param conn Estrutura de conexão do bluetooth.
* @param reason Razão de desconexão.
*/
static void disconnected_callback(struct bt_conn *conn, uint8_t reason) {
    int err;

    if(self.default_conn != NULL) {
        bt_conn_unref(self.default_conn);
        self.default_conn = NULL;
    }

    init_advertising();
}

int ble_peripheral() {
    int err;

    bt_conn_cb_register(&self.conn_callback);
    bt_gatt_cb_register(&self.gatt_callback);

    err = bt_enable(NULL);
    if (err) {
        printk("Falha ao iniciar o bluetooth. Valor: %d.\n", err);
        return -1;
    }

    printk("Bluetooth inicializado.\n");

    init_advertising();

    return 0;
}