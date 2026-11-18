#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "wifi.h"
#include "bacnet.h"
#include "modem.h"

LOG_MODULE_REGISTER(main);

#define WIFI_STACK_SIZE   4096
#define BACNET_STACK_SIZE 4096
#define MODEM_STACK_SIZE     4096

#define NORMAL_PRIORITY     5

K_THREAD_DEFINE(wifi_thread_id,
                WIFI_STACK_SIZE,
                wifi_task,
                NULL, NULL, NULL,
                NORMAL_PRIORITY, 0, 0);

K_THREAD_DEFINE(bacnet_thread_id,
                BACNET_STACK_SIZE,
                bacnet_task,
                NULL, NULL, NULL,
                NORMAL_PRIORITY, 0, 0);

K_THREAD_DEFINE(MODEM_thread_id,
                MODEM_STACK_SIZE,
                modem_task,
                NULL, NULL, NULL,
                NORMAL_PRIORITY, 0, 0);

int main(void)
{
    // Private variable declaration

    /*int wifi_status_init;
    int bacnet_status_init;

    LOG_INF("Demarrage esp");

    // Init the wifi
    LOG_INF("wifi po init");
    wifi_status_init = wifi_init();
    LOG_INF("wifi init");

    // Init bacnet device
    bacnet_status_init = bacnet_esp32_init();

    bacnet_esp32_process();*/

    return 0;
}
