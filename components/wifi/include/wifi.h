#ifndef WIFI_H
#define WIFI_H

#include "zephyr/net/wifi.h"

/**
 * @brief Establish a Wi-Fi connection to an existing access point.
 *
 * Configures the ESP32 Wi-Fi interface in Station (STA) mode
 * and requests a connection using the configured network credentials.
 *
 * @return 0 on success, negative error code otherwise.
 */
int wifi_connection(void);


/**
 * @brief Disconnect the ESP32 from the current Wi-Fi network.
 *
 * Requests disconnection from the currently connected access point.
 *
 * @return 0 on success, negative error code otherwise.
 */
int wifi_disconnection(void);


/**
 * @brief Enable the ESP32 Wi-Fi access point.
 *
 * Configures the ESP32 in Access Point (AP) mode, allowing
 * external devices to connect to the Gateway IoT.
 *
 * @return 0 on success, negative error code otherwise.
 */
int wifi_hotspot_enable(void);


/**
 * @brief Disable the ESP32 Wi-Fi access point.
 *
 * Stops the currently active Wi-Fi hotspot and prevents
 * new devices from connecting.
 *
 * @return 0 on success, negative error code otherwise.
 */
int wifi_hotspot_disable(void);


/**
 * @brief Configure the Wi-Fi power management mode.
 *
 * Enables or disables Wi-Fi Power Save mode to reduce
 * radio power consumption.
 *
 * @param i_power_mode Power mode selection:
 *                     - false: Power Save enabled.
 *                     - true:  Normal power mode.
 *
 * @return 0 on success, negative error code otherwise.
 */
int wifi_set_power_mode(bool i_power_mode);

void wifi_task(void *p1, void *p2, void *p3);
#endif
