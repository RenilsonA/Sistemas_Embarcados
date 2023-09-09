#include "ble_central.h"

static void mtu_callback(struct bt_conn *conn, uint16_t tx, uint16_t rx);
static void connected_callback(struct bt_conn *connection, uint8_t error);
static void disconnected_callback(struct bt_conn *connection, uint8_t reason);

static struct ble_t {
    bool task_flag;                                     /** Booleano que indica quando posso colocar a próxima string */
    uint16_t uart_write;                                /** Write da UART para o serviço */
    struct bt_conn_cb conn_callback;                    /** Bluetooth callback de conexões */
    struct bt_gatt_cb gatt_callback;                    /** Bluetooth callback GATT */
    struct bt_conn *default_conn;                       /** Estrutura de conexões do bluetooth*/
    struct bt_uuid_16 uuid;                             /** UUID para servico da UART */
    struct bt_gatt_discover_params discover_params;     /** Parâmetros GATT para descobrir dispositivos */
    struct bt_gatt_subscribe_params subscribe_params;   /** Parâmetros GATT para se inscrever nos dispositivos */
} self = {
    .task_flag = false,
    .conn_callback = {
        .connected    = connected_callback,
        .disconnected = disconnected_callback,
    },
    .gatt_callback = {
        .att_mtu_updated = mtu_callback,
    },
    .default_conn = NULL,
    .discover_params = {0},
    .uuid = BT_UUID_INIT_16(0),
    .subscribe_params = {0},
    .uart_write = 0,
};

static void update_timer_handler(struct k_timer *timer_id) {
    self.task_flag = true;
}

K_TIMER_DEFINE(update_timer, update_timer_handler, NULL);

/**
 * @brief Callback para descobrimento de dispositivo.
 * @param addr Ponteiro para o endereço do dispositivo encontrado.
 * @param rssi RSSI.
 * @param type Tipo de evento.
 * @param ad Ponteiro para a publicidade do dispositivo.
 */
static void device_handler(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    if (type != BT_GAP_ADV_TYPE_ADV_IND && type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
        return;
    }

    if (rssi < -70) {
        return;
    }
   
    if (bt_le_scan_stop()) {
		return;
	}

	int err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT, &self.default_conn);
	if (err) {
		printk("Falha ao se conectar. Valor do erro: %d.\n", addr_str, err);
		start_central_scan();
	}
    self.task_flag = true;
}

int start_central_scan() {
    int err;
    struct bt_le_scan_param scanParameters = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };
    err = bt_le_scan_start(&scanParameters, device_handler);
    if (err) {
        printk("Erro para iniciar o scan. Valor: %d\n", err);
    } else {
       printk("Escaneamento comecou\n");
    }
    return err;
}

/**
* @brief Callback para notificações GATT. function to handle BLE GATT notifications.
* @param conn Estrutura de conexão.
* @param params Ponteiro para parâmetros de assinatura.
* @param buf Ponteiro que contém os dados recebidos.
* @param length Tamanho dos dados recebidos.
*/
static void central_notification_handler(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *buffer, uint16_t length) {
    if (!buffer) {
        printk("Inscricao cancelada.\n");
        params->value_handle = 0;
    }
    char string[length];
    memcpy(string, buffer, length);
    string[length - 1] = '\0';
    self.task_flag = true;
    printk("Resposta recebida. Informacao: %s.\n", buffer);
    memset(buffer, '\0', length);
}

/**
 * @brief Callback para descobrir as características.
 * @param conn Estrutura de conexão.
 * @param attr Aponta para atributo GATT.
 * @param params Aponta para onde descobre os parâmetros GATT.
 */
static void discover_characteristics(struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params) {
    int err;

    if (!attr) {
        printk("Todas as caracteristicas foram descobertas.\n");
        memset(params, 0, sizeof(struct bt_gatt_discover_params));
        return;
    }

    if (!bt_uuid_cmp(self.discover_params.uuid, BT_UART_SVC_UUID)) {
        memcpy(&self.uuid, BT_UART_NOTIFY_CHAR_UUID, sizeof(self.uuid));
        self.discover_params.uuid         = &self.uuid.uuid;
        self.discover_params.start_handle = attr->handle + 1;
        self.discover_params.type         = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, &self.discover_params);
        if (err) {
            printk("Falha ao descobrir dispositivo. Valor: %d.\n", err);
        }

    } else if (!bt_uuid_cmp(self.discover_params.uuid, BT_UART_NOTIFY_CHAR_UUID)) {
        memcpy(&self.uuid, BT_UART_WRITE_CHAR_UUID, sizeof(self.uuid));
        self.discover_params.uuid          = &self.uuid.uuid;
        self.discover_params.start_handle  = attr->handle + 1;
        self.discover_params.type          = BT_GATT_DISCOVER_CHARACTERISTIC;
        self.subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

        err = bt_gatt_discover(conn, &self.discover_params);
        if (err) {
            printk("Falha ao descobrir dispositivo. Valor: %d.\n", err);
        }

    } else if (!bt_uuid_cmp(self.discover_params.uuid, BT_UART_WRITE_CHAR_UUID)) {
        memcpy(&self.uuid, BT_UUID_GATT_CCC, sizeof(self.uuid));
        self.discover_params.uuid         = &self.uuid.uuid;
        self.discover_params.start_handle = attr->handle + 1;
        self.discover_params.type         = BT_GATT_DISCOVER_DESCRIPTOR;
        self.uart_write                   = bt_gatt_attr_value_handle(attr);

        err = bt_gatt_discover(conn, &self.discover_params);
        if (err) {
            printk("Falha ao descobrir dispositivo. Valor: %d.\n", err);
        }

    } else {
        self.subscribe_params.notify     = central_notification_handler;
        self.subscribe_params.value      = BT_GATT_CCC_NOTIFY;
        self.subscribe_params.ccc_handle = attr->handle;
        err = bt_gatt_subscribe(conn, &self.subscribe_params);
        if (err && err != -EALREADY) {
            printk("Falha ao se inscrever. Valor: %d.\n", err);
        } else {
            printk("Inscrito com sucesso.\n");
        }
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
    char address[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), address, sizeof(address));
    if (err) {
        printk("Falha para conectar. Erro: %u\n", err);
        bt_conn_unref(self.default_conn);
        self.default_conn = NULL;
        start_central_scan();
        return;
    }

    if (conn == self.default_conn) {
        printk("Conectado com o dispositivo de endereco: %s\n", address);
        memcpy(&self.uuid, BT_UART_SVC_UUID, sizeof(self.uuid));
        self.discover_params.uuid         = &self.uuid.uuid;
        self.discover_params.func         = discover_characteristics;
        self.discover_params.start_handle = 0x0001;
        self.discover_params.end_handle   = 0xffff;
        self.discover_params.type         = BT_GATT_DISCOVER_PRIMARY;
        err = bt_gatt_discover(self.default_conn, &self.discover_params);
        if (err) {
            printk("Falha ao descobrir a caracteristica. Erro: %d.\n", err);
        }
    }
}

/**
* @brief Callback de desconexão.
* @param conn Estrutura de conexão do bluetooth.
* @param reason Razão de desconexão.
*/
static void disconnected_callback(struct bt_conn *conn, uint8_t reason) {
    char addr[BT_ADDR_LE_STR_LEN];
    if (conn == self.default_conn) {
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
        printk("Dispositivo %s desconectado. Razao: %x\n", addr, reason);
        self.task_flag = false;
        bt_conn_unref(self.default_conn);
        self.default_conn = NULL;
        start_central_scan();
    }
}

bool check_connection(){
    bool status = false;
    if(self.default_conn != NULL) {
        status = true;
    }
    return status;
}

bool check_task_flag(){
    return self.task_flag;
}

void send_data(char buffer[], int buffer_size) {
    int err;
    self.task_flag = false;
    if (check_connection() && !self.task_flag) {
        err = bt_gatt_write_without_response_cb(self.default_conn, self.uart_write, buffer, buffer_size, false, NULL, NULL);
        if (err) {
            self.task_flag = true;
            printk("Falha ao enviar texto. Valor: %d\n", err);
        }
        //Espere resposta por 1 segundos
        k_timer_start(&update_timer, K_SECONDS(1), K_SECONDS(1));
    } else {
        self.task_flag = true;
        printk("Conecte a um dispositivo primeiro.\n");
    }
}

int ble_central_init() {
    int err;

    //Registrando callbacks
    bt_conn_cb_register(&self.conn_callback);
    bt_gatt_cb_register(&self.gatt_callback);

    //Iniciando de fato o bluetooth
    err = bt_enable((bt_ready_cb_t) start_central_scan);
    if (err) {
        printk("Erro ao iniciar BLE, valor: %d\n", err);
        return -1;
    }
    
    printk("BLE iniciado com sucesso.\n");
    return 0;
}