/**
  **********************************************************************************************************************
  * @file    ble_gap.c
  * @brief   This file manages BLE GAP layer
  * @authors patrykmonarcha
  * @date Jan 23, 2025
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include "ble_gap.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "ble_gatt.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "BLE_GAP";

static uint8_t own_addr_type;

/* External variables ------------------------------------------------------------------------------------------------*/
uint16_t conn_handle;
uint8_t temperature_notification_enabled;
uint8_t humidity_notification_enabled;
uint8_t pressure_notification_enabled;
uint8_t audio_notification_enabled;
uint16_t temperature_notify_handle;
uint16_t humidity_notify_handle;
uint16_t pressure_notify_handle;
uint16_t audio_notify_handle;

/* Private function declarations -------------------------------------------------------------------------------------*/
/*
 * @function bleprph_advertise
 *
 * @abstract This function enables advertising in General discoverable mode/Undirected connectable mode.
 *
 * @param None
 *
 * @return None
 */
static void bleprph_advertise(void);

/*
 * @function bleprph_gap_event
 *
 * @abstract This function is a callback that gets called when a GAP event occurs. The
   application associates a GAP event callback with each connection that forms.
   bleprph uses the same callback for all connections.
 *
 * @param[in] event: BLE GAP event structure
 *
 * @param[in] arg: Application-specified argument; unused by bleprph
 *
 * @return
 *  - 0 if the application successfully handled the event
 *  - nonzero on failure
 */
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);

/*
 * @function bleprph_on_reset
 *
 * @abstract This function prints the textual representation of BLE reset reason
 *
 * @param[in] reason: BLE reset reason ID
 *
 * @return None
 */
static void bleprph_on_reset(int reason);

/*
 * @function bleprph_on_sync
 *
 * @abstract This function allows the device to advertise itself over BLE
 *
 * @param None
 *
 * @return None
 */
static void bleprph_on_sync(void);

/*
 * @function subscribe_event
 *
 * @abstract This function prints logs regarrding subscribing to the characteristics
 *
 * @param[in] attr_handle: Handle to the characteristic
 *
 * @param[in] curr_notify: Notification's subscription status
 *
 * @return None
 */
static void subscribe_event(uint16_t attr_handle, uint8_t curr_notify);

/* Private function definitions --------------------------------------------------------------------------------------*/
static void bleprph_advertise(void) {

    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;
    fields.uuids16 = (ble_uuid16_t[]) {
            BLE_UUID16_INIT(DEVICE_SVR_SVC_UUID)
    };

    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error enabling advertisement; rc=%d\n", rc);
        return;
    }

}

static int bleprph_gap_event(struct ble_gap_event *event, void *arg) {

    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            /* A new connection was established or a connection attempt failed. */
            ESP_LOGD(TAG, "Connection %s; status=%d \n",
                        event->connect.status == 0 ? "established" : "failed",
                        event->connect.status);

            if (event->connect.status == 0) {
                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                assert(rc == 0);
            }

            rc = ble_att_set_preferred_mtu(512);

            if (rc != 0) {
                ESP_LOGE(TAG, "Failed to set preferred MTU; rc = %d", rc);
            }

            rc = ble_gattc_exchange_mtu(event->connect.conn_handle, NULL, NULL);

            if (rc != 0) {
                ESP_LOGE(TAG, "Failed to negotiate MTU; rc = %d", rc);
            }


            if (event->connect.status != 0) {
                /* Connection failed; resume advertising. */
                bleprph_advertise();
            }


            conn_handle = event->connect.conn_handle;

            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGD(TAG, "Disconnect; reason=%d \n", event->disconnect.reason);

            temperature_notification_enabled = 0;
            humidity_notification_enabled = 0;
            pressure_notification_enabled = 0;
            audio_notification_enabled = 0;

            /* Connection terminated; resume advertising. */
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            /* The central has updated the connection parameters. */
            ESP_LOGD(TAG, "Connection updated; status=%d \n",
                        event->conn_update.status);
            rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
            assert(rc == 0);
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGD(TAG, "Advertise complete; reason=%d\n",
                        event->adv_complete.reason);
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_ENC_CHANGE:
            /* Encryption has been enabled or disabled for this connection. */
            ESP_LOGD(TAG, "Encryption change event; status=%d \n",
                        event->enc_change.status);
            rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
            assert(rc == 0);
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:
            ESP_LOGD(TAG, "Subscribe event; conn_handle=%d attr_handle=%d "
                              "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                        event->subscribe.conn_handle,
                        event->subscribe.attr_handle,
                        event->subscribe.reason,
                        event->subscribe.prev_notify,
                        event->subscribe.cur_notify,
                        event->subscribe.prev_indicate,
                        event->subscribe.cur_indicate);

            subscribe_event(event->subscribe.attr_handle, event->subscribe.cur_notify);

            return 0;

        case BLE_GAP_EVENT_MTU:
            ESP_LOGD(TAG, "MTU update event; conn_handle=%d cid=%d mtu=%d\n",
                        event->mtu.conn_handle,
                        event->mtu.channel_id,
                        event->mtu.value);
            return 0;

        case BLE_GAP_EVENT_REPEAT_PAIRING:
            /* We already have a bond with the peer, but it is attempting to
             * establish a new secure link.  This app sacrifices security for
             * convenience: just throw away the old bond and accept the new link.
             */

            /* Delete the old bond. */
            rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            assert(rc == 0);
            ble_store_util_delete_peer(&desc.peer_id_addr);

            /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
             * continue with the pairing operation.
             */
            return BLE_GAP_REPEAT_PAIRING_RETRY;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            ESP_LOGD(TAG, "PASSKEY_ACTION_EVENT started \n");
            struct ble_sm_io pkey = {0};
            //int key = 0;

            if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
                pkey.action = event->passkey.params.action;
                pkey.passkey = 123456; // This is the passkey to be entered on peer
                ESP_LOGD(TAG, "Enter passkey %lu on the peer side", pkey.passkey);
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                ESP_LOGD(TAG, "ble_sm_inject_io result: %d\n", rc);
            } else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
                ESP_LOGD(TAG, "Passkey on device's display: %lu", event->passkey.params.numcmp);
                ESP_LOGD(TAG, "Accept or reject the passkey through console in this format -> key Y or key N");
                pkey.action = event->passkey.params.action;
                pkey.numcmp_accept = 0;
                ESP_LOGE(TAG, "Timeout! Rejecting the key");
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                ESP_LOGD(TAG, "ble_sm_inject_io result: %d\n", rc);
            } else if (event->passkey.params.action == BLE_SM_IOACT_OOB) {
                static uint8_t tem_oob[16] = {0};
                pkey.action = event->passkey.params.action;
                for (int i = 0; i < 16; i++) {
                    pkey.oob[i] = tem_oob[i];
                }
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                ESP_LOGD(TAG, "ble_sm_inject_io result: %d\n", rc);
            } else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
                ESP_LOGD(TAG, "Enter the passkey through console in this format-> key 123456");
                pkey.action = event->passkey.params.action;
                pkey.passkey = 0;
                ESP_LOGE(TAG, "Timeout! Passing 0 as the key");
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                ESP_LOGD(TAG, "ble_sm_inject_io result: %d\n", rc);
            }
            return 0;
    }

    return 0;

}

static void bleprph_on_reset(int reason) {

    ESP_LOGE(TAG, "Resetting state; reason=%d\n", reason);

}

static void bleprph_on_sync(void) {

    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    /* Begin advertising. */
    bleprph_advertise();

}

static void subscribe_event(uint16_t attr_handle, uint8_t curr_notify) {

    if (attr_handle == temperature_notify_handle) {
        temperature_notification_enabled = curr_notify;
    } else if (attr_handle == humidity_notify_handle) {
        humidity_notification_enabled = curr_notify;
    } else if (attr_handle == pressure_notify_handle) {
        pressure_notification_enabled = curr_notify;
    } else if (attr_handle == audio_notify_handle) {
        audio_notification_enabled = curr_notify;
    } else {

    }

}
/* Exported function declarations ------------------------------------------------------------------------------------*/
void ble_store_config_init(void);

/* Exported function definitions -------------------------------------------------------------------------------------*/
void bleprph_host_task(void *param) {

    ESP_LOGI(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();

}

void ble_init(void) {

    temperature_notification_enabled = 0;
    humidity_notification_enabled = 0;
    pressure_notification_enabled = 0;
    audio_notification_enabled = 0;

    int rc;

    /* Initialize NVS — it is used to store PHY calibration data */
    ESP_LOGI(TAG, "Init NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(nimble_port_init());
    ESP_LOGI(TAG, "Nimble init");

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = bleprph_on_reset;
    ble_hs_cfg.sync_cb = bleprph_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = 3;
#ifdef CONFIG_EXAMPLE_BONDING
    ble_hs_cfg.sm_bonding = 1;
#endif
#ifdef CONFIG_EXAMPLE_MITM
    ble_hs_cfg.sm_mitm = 1;
#endif
#ifdef CONFIG_EXAMPLE_USE_SC
    ble_hs_cfg.sm_sc = 1;
#else
    ble_hs_cfg.sm_sc = 0;
#endif
#ifdef CONFIG_EXAMPLE_BONDING
    ble_hs_cfg.sm_our_key_dist = 1;
    ble_hs_cfg.sm_their_key_dist = 1;
#endif

    ESP_LOGI(TAG, "GATT SVR init");
    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name. */
    ESP_LOGI(TAG, "Set device name to %s", DEVICE_NAME);
    rc = ble_svc_gap_device_name_set(DEVICE_NAME);
    assert(rc == 0);

    /* XXX Need to have template for store */
    ESP_LOGI(TAG, "Store config");
    ble_store_config_init();
    ESP_LOGI(TAG, "Nimble task init");
    nimble_port_freertos_init(bleprph_host_task);

}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
