/**
  **********************************************************************************************************************
  * @file    ble_gatt.h
  * @abstract   This file is the header file for BLE GATT
  * @authors patrykmonarcha
  * @date Jan 23, 2025
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef _BLE_GATT_H_
#define _BLE_GATT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------------------*/
#include <stdbool.h>

/* Types ----------------------------------------------------------------------------------------------------*/

/* External Variables ------------------------------------------------------------------------------------------------*/

/* Constants ------------------------------------------------------------------------------------------------*/
/** @abstract BLE advertise service UUID */
#define DEVICE_SVR_SVC_UUID 0x0011

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/
/** @abstract BLE temperature notification handle */
extern uint16_t temperature_notify_handle;
/** @abstract BLE humidity notification handle */
extern uint16_t humidity_notify_handle;
/** @abstract BLE pressure notification handle */
extern uint16_t pressure_notify_handle;
/** @abstract BLE audio notification handle */
extern uint16_t audio_notify_handle;

/* Functions ------------------------------------------------------------------------------------------------*/
/*
 * @function gatt_svr_init
 *
 * @abstract This function initializes GATT server
 *
 * @param None
 *
 * @return 0 on success
 */
int gatt_svr_init(void);

/*
 * @function send_temperature_notification
 *
 * @abstract This function is used to send a notification with an information regarding temperature
 *
 * @param None
 *
 * @return None
 */
void send_temperature_notification(void);

/*
 * @function send_humidity_notification
 *
 * @abstract This function is used to send a notification with an information regarding humidity
 *
 * @param  None
 *
 * @return None
 */
void send_humidity_notification(void);

/*
 * @function send_pressure_notification
 *
 * @abstract This function is used to send a notification with an information regarding pressure
 *
 * @param  None
 *
 * @return None
 */
void send_pressure_notification(void);

/*
 * @function send_audio_notification
 *
 * @abstract This function is used to send a notification with an information regarding audio
 *
 * @param  None
 *
 * @return None
 */
void send_audio_notification(void);

#ifdef __cplusplus
}
#endif

#endif // _BLE_GATT_H_

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
