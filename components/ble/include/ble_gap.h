/**
  **********************************************************************************************************************
  * @file    ble_gap.h
  * @brief   This file is the header file for BLE GAP
  * @authors patrykmonarcha
  * @date Jan 23, 2025
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef _BLE_GAP_H_
#define _BLE_GAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "nimble/ble.h"

/* Types ----------------------------------------------------------------------------------------------------*/
struct ble_hs_cfg;

struct ble_gatt_register_ctxt;

/* Constants ------------------------------------------------------------------------------------------------*/
/** @abstract BLE device name on advertisment */
#define DEVICE_NAME "ID-169"

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/
/** @abstract BLE connection handle */
extern uint16_t conn_handle;
/** @abstract Flag storing temperature notifications characteristic subscription state */
extern uint8_t temperature_notification_enabled;
/** @abstract Flag storing humidity notifications characteristic subscription state */
extern uint8_t humidity_notification_enabled;
/** @abstract Flag storing pressure notifications characteristic subscription state */
extern uint8_t pressure_notification_enabled;
/** @abstract Flag storing audio notifications characteristic subscription state */
extern uint8_t audio_notification_enabled;

/* Functions ------------------------------------------------------------------------------------------------*/
/*
 * @function gatt_svr_register_cb
 *
 * @abstract This function manages callbacks for registering BLE services, characteristics and descriptors
 *
 * @param[in] ctxt: GATT context
 *
 * @param[in] arg: Generic pointer
 *
 * @return None
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);

/*
 * @function ble_init
 *
 * @abstract This function initializes BLE
 *
 * @param None
 *
 * @return None
 */
void ble_init(void);

/*
 * @function bleprph_host_task
 *
 * @abstract This function starts NimBLE task
 *
 * @param[in] param: Generic pointer
 *
 * @return None
 */
void bleprph_host_task(void *param);

#ifdef __cplusplus
}
#endif

#endif // _BLE_GAP_H_

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
