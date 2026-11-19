#include "modem.h"
#include "zephyr/sys/util_macro.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

#define MODEM_UART_NODE DT_NODELABEL(uart1)

static const struct device *modem_uart = DEVICE_DT_GET(MODEM_UART_NODE);

LOG_MODULE_REGISTER(MODEM);

int modem_init(void)
{
    // Private variable declaration
    int ret;

    //Verify the UART device initialized
    if (!device_is_ready(modem_uart))
	{
	    // UART failed
        LOG_INF("UART device %s is not ready\n", modem_uart->name);
    }
    else
	{
	    // UART ready
        LOG_INF("UART device %s is ready\n", modem_uart->name);
    }

    // UART config
    const struct uart_config uart_cfg = {
        .baudrate = 115200,
        .parity = UART_CFG_PARITY_NONE,
        .stop_bits = UART_CFG_STOP_BITS_1,
        .data_bits = UART_CFG_DATA_BITS_8,
        .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
    };

    // Set the UART config and check if it's successfully set
    ret = uart_configure(modem_uart, &uart_cfg);

    //Check is the config is set
    if (ret != 0)
	{
        // Config not set
        LOG_ERR("Failed to configure UART (err: %d)\n", ret);
        return ret;
    }
    else
	{
	    // UART ready
        LOG_INF("UART successfully configure");
    }
    return 0;
}

void modem_send_at(const char *command)
{
    // Loop on the command to sens
    for (int i = 0; command[i] != '\0'; i++)
	{
        // Send character on UART
        uart_poll_out(modem_uart, command[i]);
    }

    // Send carriage return
    uart_poll_out(modem_uart, '\r');
}

void modem_read_response(void)
{
    unsigned char c;
    int ret;

        ret = uart_poll_in(modem_uart, &c);

        if (ret == 0) {
            LOG_INF("Received: 0x%02X", c);
        } else {
            LOG_ERR("NO RECEIVE");
        }
}

// task cauldron just to check if modem 4G function work
void modem_task(void *p1, void *p2, void *p3)
{
	ARGS_UNUSED(p1);
	ARGS_UNUSED(p2);
	ARGS_UNUSED(p3);
    modem_init();
    k_sleep(K_SECONDS(5));

    while (1)
	{
        modem_send_at("AT");
        LOG_INF("Command sent successfully");
        k_msleep(10);
        modem_read_response();

        k_sleep(K_SECONDS(1));
        k_sleep(K_SECONDS(1));
        k_sleep(K_SECONDS(1));
        k_sleep(K_SECONDS(1));
        k_sleep(K_SECONDS(1));
        k_sleep(K_SECONDS(1));

    }
}
