#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/logging/log.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* BACnet Stack : définitions de base, à inclure en premier */
#include "bacnet/bacdef.h"
#include "bacnet/version.h"

/* Timers */
#include "bacnet/basic/sys/mstimer.h"

/* Device et objets BACnet */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/ai.h"

#if (BACNET_PROTOCOL_REVISION >= 17)
#include "bacnet/basic/object/netport.h"
#endif

/* Serveur BACnet */
#include "bacnet/basic/server/bacnet_basic.h"
#include "bacnet/basic/server/bacnet_port.h"

/* Configuration et sauvegarde des propriétés */
#include <bacnet_settings/bacnet_settings.h>
#include <bacnet_settings/bacnet_storage.h>

/* Réinitialisation du device */
#include <bacnet_osif/bacnet_reinit.h>

/* Logs BACnet */
#include <bacnet_osif/bacnet_log.h>

#include "bacnet.h"

LOG_MODULE_DECLARE(bacnet, CONFIG_BACNETSTACK_LOG_LEVEL);

/* Default values before we get the device instance and name from settings */
static const char *Device_Name = "myesp32";
static const uint32_t Device_Instance = 260121;
/* object instances */
static const uint32_t Sensor_Instance = 1;

/* timer for Sensor Update Interval */
static struct mstimer Sensor_Update_Timer;

/**
 * @brief Clear any stored BACnet settings before a cold start reboot
 * @param context [in] The context to pass to the callback function
 */
static void BACnet_Smart_Sensor_Coldstart_Callback(void *context)
{
    int err;

    (void)context;
    LOG_INF("COLDSTART: Clearing BACnet settings...");
    err = bacnet_settings_clear();
    if (err < 0) {
        LOG_ERR("COLDSTART: Failed to clear BACnet settings: %d", err);
    } else {
        LOG_INF("COLDSTART: Successfully cleared BACnet settings");
    }
}

/**
 * @brief Callback data for WriteProperty restore iterator
 * @param write_function The WriteProperty function to call
 * @param context The context to pass to the WriteProperty function
 * @return true if the WriteProperty succeeded
 */
static bool
Settings_Restore_Callback(BACNET_WRITE_PROPERTY_DATA *wp_data, void *context)
{
    (void)context;
    return Device_Write_Property(wp_data);
}

/**
 * @brief BACnet Project Initialization Handler
 * @param context [in] The context to pass to the callback function
 * @note This is called from the BACnet task
 */
static void BACnet_Smart_Sensor_Init_Handler(void *context)
{
    const float default_temperature = 25.0f;

    (void)context;
    LOG_INF("BACnet Stack Initialized");
    /* initialize objects with default values for this basic sample */
    Device_Set_Object_Instance_Number(Device_Instance);
    Device_Object_Name_ANSI_Init(Device_Name);
    Analog_Input_Create(Sensor_Instance);
    Analog_Input_Name_Set(Sensor_Instance, "Sensor");
    Analog_Input_Present_Value_Set(Sensor_Instance, default_temperature);
    Analog_Input_Units_Set(Sensor_Instance, UNITS_DEGREES_CELSIUS);
    Analog_Input_COV_Increment_Set(Sensor_Instance, 1.0f);
    /* restore any property values previously stored via WriteProperty */
    bacnet_settings_init();
    bacnet_settings_write_property_restore(&Settings_Restore_Callback, NULL);
    /* writable property values are stored with WriteProperty.
       Set this callback after restore to prevent recursion. */
    bacnet_basic_store_callback_set(bacnet_settings_basic_store);
    LOG_INF("BACnet Device ID: %u", Device_Object_Instance_Number());
    /* start the seconds cyclic timer */
    mstimer_set(&Sensor_Update_Timer, 1000);
    bacnet_reinitialize_device_init(CONFIG_BACNET_REINIT_REBOOT_DELAY);
    srand(sys_rand32_get());
}

/**
 * @brief BACnet Project Task Handler
 * @param context [in] The context to pass to the callback function
 * @note This is called from the BACnet task
 */
static void BACnet_Smart_Sensor_Task_Handler(void *context)
{
    float temperature = 0.0f, change = 0.0f;

    bacnet_reinitialize_device_task(
        BACnet_Smart_Sensor_Coldstart_Callback, context);
    if (mstimer_expired(&Sensor_Update_Timer)) {
        mstimer_reset(&Sensor_Update_Timer);
        /* simulate a sensor reading, and update the BACnet object values */
        if (Analog_Input_Out_Of_Service(Sensor_Instance)) {
            return;
        }
        temperature = Analog_Input_Present_Value(Sensor_Instance);
        change = -1.0f + 2.0f * ((float)sys_rand32_get() / (float)UINT32_MAX);
        temperature += change;
        Analog_Input_Present_Value_Set(Sensor_Instance, temperature);
    }
}

static bool port_initialized = false;

int bacnet_esp32_init(void)
{
    LOG_INF("BACnet Device: %s", Device_Name);
    LOG_INF("BACnet Stack Version " BACNET_VERSION_TEXT);
    LOG_INF("BACnet Stack Max APDU: %d", MAX_APDU);
    bacnet_basic_init_callback_set(BACnet_Smart_Sensor_Init_Handler, NULL);
    bacnet_basic_task_callback_set(BACnet_Smart_Sensor_Task_Handler, NULL);
    bacnet_basic_init();

    return 0;
}

void bacnet_task(void *p1, void *p2, void *p3)
{

	ARGS_UNUSED(p1);
	ARGS_UNUSED(p2);
	ARGS_UNUSED(p3);

    bacnet_esp32_init();

    // Loop to ensure the network port is successfully init
    while (1) {
        // Bacnet process
        bacnet_basic_task();

        // Check if the network port is initialized
        if (port_initialized)
        {
            // Paket processing
            bacnet_port_task();
        }

        else
        {
            // Network port init
            port_initialized = bacnet_port_init();
        }

        // Config bacnet delay
        k_sleep(K_MSEC(CONFIG_BACNET_BASIC_SERVER_KSLEEP));
    }
}
