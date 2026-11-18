#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>
#include "wifi.h"
#include "zephyr/net/wifi.h"
#include "zephyr/sys/util_macro.h"

LOG_MODULE_REGISTER(WIFI);

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

#define NET_EVENT_WIFI_MASK \
    (NET_EVENT_WIFI_CONNECT_RESULT | \
     NET_EVENT_WIFI_DISCONNECT_RESULT | \
     NET_EVENT_WIFI_SCAN_RESULT)


static struct net_if *sta_iface;

static struct wifi_connect_req_params sta_config;

static struct net_mgmt_event_callback cb;

static bool wifi_connected;

/* Check necessary definitions */

BUILD_ASSERT(sizeof(CONFIG_WIFI_SAMPLE_SSID) > 1,
	     "CONFIG_WIFI_SAMPLE_SSID is empty. Please set it in conf file.");


static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
			       struct net_if *iface)
{
    // Different wifi event
	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
       // Get the status result of the wifi connection
       const struct wifi_status *status_res = (const struct wifi_status *) cb -> info ;

       // Check the status of the wifi connection
       if (status_res->conn_status == WIFI_STATUS_CONN_SUCCESS) {
           // Connected
           LOG_INF("Connected to %s", CONFIG_WIFI_SAMPLE_SSID);
           wifi_connected = true;
       }
       else {
           // Connection failed
           LOG_ERR("FAILED for the following reason : %s",wifi_conn_status_txt(status_res->conn_status));
       }

	break;

	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		LOG_INF("Disconnected from %s", CONFIG_WIFI_SAMPLE_SSID);
		break;

    case NET_EVENT_WIFI_SCAN_RESULT:
       const struct wifi_scan_result *scan_res = (const struct wifi_scan_result *) cb -> info ;

		LOG_INF("ssid : %s | rssi : %i | band : %i | security : %s", scan_res->ssid, scan_res->rssi, scan_res->band, wifi_security_txt(scan_res->security));

		break;

	default:
		break;
	}
}

static int connect_to_wifi(void)
{
	if (!sta_iface) {
		LOG_INF("STA: interface no initialized");
		return -EIO;
	}

	sta_config.ssid = (const uint8_t *)CONFIG_WIFI_SAMPLE_SSID;
	sta_config.ssid_length = sizeof(CONFIG_WIFI_SAMPLE_SSID) - 1;
	sta_config.psk = (const uint8_t *)CONFIG_WIFI_SAMPLE_PSK;
	sta_config.psk_length = sizeof(CONFIG_WIFI_SAMPLE_PSK) - 1;
	sta_config.security = WIFI_SECURITY_TYPE_PSK;
	sta_config.channel = WIFI_CHANNEL_ANY;
	sta_config.band = WIFI_FREQ_BAND_2_4_GHZ;

	LOG_INF("Connecting to SSID: %s\n", sta_config.ssid);

	int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, sta_iface, &sta_config,
			   sizeof(struct wifi_connect_req_params));
	if (ret) {
		LOG_ERR("Unable to Connect to (%s)", CONFIG_WIFI_SAMPLE_SSID);
	}

	return ret;
}

// Wifi initialization function
int wifi_connection(void)
{
	wifi_connected = false;
	k_sleep(K_SECONDS(5));

    // Init the event callback and assiociate it with wifi_event_handler
	net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);

    // Add event callback
	net_mgmt_add_event_callback(&cb);

	// Get STA interface in AP-STA mode
	sta_iface = net_if_get_wifi_sta();

    // Scan the area
    int ret = net_mgmt(NET_REQUEST_WIFI_SCAN, sta_iface, NULL, 0);

    // Ensure the scan is successfully done
    if (ret) {

        // Scan failed
        LOG_ERR("Erreur scan : %d\n", ret);
    }

    k_msleep(5000);

    while (!wifi_connected)
	{
        connect_to_wifi();
        k_sleep(K_SECONDS(5));
    }

	return 0;
}

// Wifi disconnection function
int wifi_disconnection(void)
{
    int ret = net_mgmt(NET_REQUEST_WIFI_DISCONNECT,
                   sta_iface,
                   NULL,
                   0);

    if (ret) {
        LOG_ERR("WiFi disconnection failed: %d", ret);
        return ret;
    }


	wifi_connected = false;
    LOG_INF("WiFi disconnected");

    return 0;
}

// Create a hotspot wifi on esp32
int wifi_hotspot_enable(void)
{
    // Access point definition
    static const uint8_t ap_ssid[] = "Gateway_IoT_BANKSSSSSS";
    static const uint8_t ap_psk[] = "mathistropfor";

    // Hotspot setting struct
    struct wifi_connect_req_params ap_config = {0};

    // Struct definition
    ap_config.ssid = ap_ssid;
    ap_config.ssid_length = sizeof(ap_ssid) - 1;

    // Password configuration
    ap_config.psk = ap_psk;
    ap_config.psk_length = sizeof(ap_psk) - 1;

    // Channel and band configuration
    ap_config.band = WIFI_FREQ_BAND_2_4_GHZ;
    ap_config.channel = WIFI_CHANNEL_ANY;

	// Get STA interface in AP-STA mode
    sta_iface = net_if_get_wifi_sta();

    // Wifi security
    ap_config.security = WIFI_SECURITY_TYPE_PSK;

    int ret = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE,
                       sta_iface,
                       &ap_config,
                       sizeof(ap_config));

    // Result verification
    if (ret) {
        LOG_ERR("Hotspot activation failed: %d", ret);
        return ret;
    }

    LOG_INF("Hotspot activation requested");

    return 0;
}

// Function to disable hotspot wifi
int wifi_hotspot_disable(void)
{
    int ret = net_mgmt(NET_REQUEST_WIFI_AP_DISABLE,
                   sta_iface,
                   NULL,
                   0);

    if (ret) {
        LOG_ERR("Hotspot disable failed: %d", ret);
        return ret;
    }

    LOG_INF("Hotspot disabled");

    return 0;
}


// function to set the power mode 0 for power saving mode and 1 to normal power mode
int wifi_set_power_mode(bool i_power_mode)
{
    // private params declaration
    struct wifi_ps_params ps_params = {0};

    // Check if the normal power mode is requested
    if (i_power_mode)
	{
	    // set the parameter to power saving disable
        ps_params.enabled = WIFI_PS_DISABLED;
    }

    else
	{
	    // set the parameter to power saving enable
        ps_params.enabled = WIFI_PS_ENABLED;
    }

    int ret = net_mgmt(NET_REQUEST_WIFI_PS,
                       sta_iface,
                       &ps_params,
                       sizeof(ps_params));

    if (ret) {
        LOG_ERR("Can't change current mode: %d", ret);
        return ret;
    }

    LOG_INF("Wi-Fi power mode successfully changed to: %s",
            i_power_mode ? "NORMAL" : "POWER SAVING");

    return 0;
}

// task cauldron just to check if wifi function work
void wifi_task(void *p1, void *p2, void *p3)
{
	ARGS_UNUSED(p1);
	ARGS_UNUSED(p2);
	ARGS_UNUSED(p3);

    wifi_connection();
    wifi_hotspot_enable();

    k_sleep(K_SECONDS(30));
    wifi_set_power_mode(0);
    wifi_disconnection();

    while (1) {
        // Gestion du Wi-Fi
        k_sleep(K_SECONDS(1));
    }
}
