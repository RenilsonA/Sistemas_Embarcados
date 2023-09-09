#include "ble_central.h"

static void input_task(void) {
    char *input;
    console_getline_init();
    while (true) {
        //Se estiver conectado a algo, e não estou esperando notificação:
        if(check_connection() && check_task_flag()) {
            printk("Insira algum texto: ");
            input = console_getline();
            if (input != NULL) {
                printk("Enviando: %s\n", input);
                send_data(input, strlen(input));
            } else {
                printk("Entrada vazia!\n");
            }
        }
    }
}

void main(void) {
    printk("Iniciando central.\n");
    ble_central_init();
}

/** @brief Thread object to handle user input */
K_THREAD_DEFINE(input, 2048, input_task, NULL, NULL, NULL, 1, 0, 1000);