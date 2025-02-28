/**
  **********************************************************************************************************************
  * @file    ble_gatt.c
  * @brief   This file manages BLE GATT layer
  * @authors patrykmonarcha
  * @date Jan 23, 2025
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_gap.h"
#include "ble_gatt.h"
#include "rtc_driver.h"

/* Private define ----------------------------------------------------------------------------------------------------*/
#define DEVICE_MODEL_NUMBER "ID-169"
#define DEVICE_SERIAL_NUMBER "S/N 001"
#define DEVICE_FIRMWARE_REVISION "1.0.0"
#define DEVICE_MANUFACTURER_NAME "Politechnika Gdańska"

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "BLE_GATT";

// 18 0A
/** @abstract BLE Device Information Service UUID */
static const ble_uuid16_t gatt_svr_svc_device_information_service_uuid = BLE_UUID16_INIT(0x180A);

// 2A 24
/** @brief BLE Model Number Characteristic UUID */
static const ble_uuid16_t gatt_svr_chr_model_number_uuid = BLE_UUID16_INIT(0x2A24);

// 2A 25
/** @brief BLE Serial Number Characteristic UUID */
static const ble_uuid16_t gatt_svr_chr_serial_number_uuid = BLE_UUID16_INIT(0x2A25);

// 2A 26
/** @brief BLE Firmware Revision Characteristic UUID */
static const ble_uuid16_t gatt_svr_chr_firmware_revision_uuid = BLE_UUID16_INIT(0x2A26);

// 2A 29
/** @brief BLE Manufacturer Name Characteristic UUID */
static const ble_uuid16_t gatt_svr_chr_manufacturer_name_uuid = BLE_UUID16_INIT(0x2A29);

// 18 05
/** @abstract BLE Current Time Service UUID */
static const ble_uuid16_t gatt_svr_svc_current_time_service_uuid = BLE_UUID16_INIT(0x1805);

// 2A 2B
/** @brief BLE Current Time Characteristic UUID */
static const ble_uuid16_t gatt_svr_chr_current_time_uuid = BLE_UUID16_INIT(0x2A2B);


// CB B6 06 7B 19 18 44 F3 89 E4 30 43 A2 D1 2E 27
/** @abstract BLE Custom Temperature Service UUID */
static const ble_uuid128_t gatt_svr_svc_custom_temperature_service_uuid =
        BLE_UUID128_INIT(0x27, 0x2E, 0xD1, 0xA2, 0x43, 0x30, 0xE4, 0x89,
                         0xF3, 0x44, 0x18, 0x19, 0x7B, 0x06, 0xB6, 0xCB);

// 07 E5 D6 F7 6F 18 4D CE BB 5B 73 2C AB 4E 84 74
/** @brief BLE Temperature Stream Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_temperature_stream_uuid =
        BLE_UUID128_INIT(0x74, 0x84, 0x4E, 0xAB, 0x2C, 0x73, 0x5B, 0xBB,
                         0xCE, 0x4D, 0x18, 0x6F, 0xF7, 0xD6, 0xE5, 0x07);

// 34 14 6E 5A 52 A7 47 AB BB 6B 34 CE EC 29 41 EE
/** @brief BLE Memory Status Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_temperature_memory_status_uuid =
        BLE_UUID128_INIT(0xEE, 0x41, 0x29, 0xEC, 0xCE, 0x34, 0x6B, 0xBB,
                         0xAB, 0x47, 0xA7, 0x52, 0x5A, 0x6E, 0x14, 0x34);

// 86 AF 1E 06 D1 A5 4A 14 A2 E9 B4 93 13 40 5E ED
/** @brief BLE Custom Humidity Service UUID */
static const ble_uuid128_t gatt_svr_chr_custom_humidity_service_uuid =
        BLE_UUID128_INIT(0xED, 0x5E, 0x40, 0x13, 0x93, 0xB4, 0xE9, 0xA2,
                         0x14, 0x4A, 0xA5, 0xD1, 0x06, 0x1E, 0xAF, 0x86);

// 97 DC E1 33 09 16 4D 5C A1 E3 21 7B 10 B3 7D 58
/** @brief BLE Humidity Stream Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_humidity_stream_uuid =
        BLE_UUID128_INIT(0x58, 0x7D, 0xB3, 0x10, 0x7B, 0x21, 0xE3, 0xA1,
                        0x5C, 0x4D, 0x16, 0x09, 0x33, 0xE1, 0xDC, 0x97);

// 1C 8F 7E C6 26 19 47 BF B0 1F 61 21 C1 D1 FF 65
/** @brief BLE Memory Status Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_humidity_memory_status_uuid =
        BLE_UUID128_INIT(0x65, 0xFF, 0xD1, 0xC1, 0x21, 0x61, 0x1F, 0xB0,
                        0xBF, 0x47, 0x19, 0x26, 0xC6, 0x7E, 0x8F, 0x1C);

// 8D CF 22 A9 F7 EF 48 CD AD FC AC B2 1B F6 1B 4B
/** @brief BLE Custom Pressure Service UUID */
static const ble_uuid128_t gatt_svr_chr_custom_pressure_service_uuid =
        BLE_UUID128_INIT(0x4B, 0x1B, 0xF6, 0x1B, 0xB2, 0xAC, 0xFC, 0xAD,
                        0xCD, 0x48, 0xEF, 0xF7, 0xA9, 0x22, 0xCF, 0x8D);

// 55 FC D9 B7 63 B3 48 20 A2 6C 73 A8 A7 BB 8D 6F
/** @brief BLE Pressure Stream Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr__pressure_stream_uuid =
        BLE_UUID128_INIT(0x6F, 0x8D, 0xBB, 0xA7, 0xA8, 0x73, 0x6C, 0xA2,
                        0x20, 0x48, 0xB3, 0x63, 0xB7, 0xD9, 0xFC, 0x55);

// A9 56 BD 16 50 28 48 D1 93 FE D4 42 E7 68 42 FF
/** @brief BLE Memory Status Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_pressure_memory_status_uuid =
        BLE_UUID128_INIT(0xFF, 0x42, 0x68, 0xE7, 0x42, 0xD4, 0xFE, 0x93,
                        0xD1, 0x48, 0x28, 0x50, 0x16, 0xBD, 0x56, 0xA9);

// FA 12 44 61 66 EA 4E 1B B1 E4 E5 8C B6 DE C6 BE
/** @brief BLE Custom Microphone Service UUID */
static const ble_uuid128_t gatt_svr_chr_custom_microphone_service_uuid =
        BLE_UUID128_INIT(0xBE, 0xC6, 0xDE, 0xB6, 0x8C, 0xE5, 0xE4, 0xB1,
                        0x1B, 0x4E, 0xEA, 0x66, 0x61, 0x44, 0x12, 0xFA);

// B0 CE 3C 07 AA 05 4C 8C 8E 89 6F 62 EC D7 DA C2
/** @brief BLE Microphone Stream Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_microphone_stream_uuid =
        BLE_UUID128_INIT(0xC2, 0xDA, 0xD7, 0xEC, 0x62, 0x6F, 0x89, 0x8E,
                        0x8C, 0x4C, 0x05, 0xAA, 0x07, 0x3C, 0xCE, 0xB0);

// 7D E1 41 6F CF 11 4E F2 86 C8 0F 40 AE B6 6A AE
/** @brief BLE Memory Status Characteristic UUID */
static const ble_uuid128_t gatt_svr_chr_microphone_memory_status_uuid =
        BLE_UUID128_INIT(0xAE, 0x6A, 0xB6, 0xAE, 0x40, 0x0F, 0xC8, 0x86,
                        0xF2, 0x4E, 0x11, 0xCF, 0x6F, 0x41, 0xE1, 0x7D);

/** @abstract Holding Temperature Stream characteristic value */
static uint8_t * gatt_svr_chr_temperature_stream_value = NULL;

/** @abstract Holding Humidity Stream characteristic value */
static uint8_t * gatt_svr_chr_humidity_stream_value = NULL;

/** @abstract Holding Pressure Stream characteristic value */
static uint8_t * gatt_svr_chr_pressure_stream_value = NULL;

/** @abstract Holding Audio Stream characteristic value */
static uint8_t * gatt_svr_chr_audio_stream_value = NULL;


/* External variables ------------------------------------------------------------------------------------------------*/


/* Private function declarations -------------------------------------------------------------------------------------*/
/**
 *
 * @brief This function copies the contents of an mbuf into the specified flat buffer
 *
 * @param[in] om The mbuf to copy from.
 *
 * @param[in] min_len  The minimum size of the flat buffer
 *
 * @param[in] max_len The maximum size of the flat buffer
 *
 * @param[out] dst The destination buffer
 *
 * @param len[in]  The number of bytes to copy
 *
 * @return 0 on successful operation
 */
static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len);

/**
 *
 * @brief This function is a callback that gets called when a GATT event occurs.
 *
 * @param[in] conn_handle: BLE connection handle
 *
 * @param[in] attr_handle: BLE attribute handle
 *
 * @param[in] ctxt: BLE context for an access to a GATT characteristic
 *
 * @param[in] arg: Unused
 *
 * @return 0 on successful operation
 */
static int gatt_svr_chr_access_all(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt,
                                   void *arg);

/* Private typedef ---------------------------------------------------------------------------------------------------*/
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &gatt_svr_svc_current_time_service_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[])
            { {
                      .uuid = &gatt_svr_chr_current_time_uuid.u,
                      .access_cb = gatt_svr_chr_access_all,
                      .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
              },
              {
                      0, /* No more characteristics in this service. */
              }
            },
    },
    {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &gatt_svr_svc_device_information_service_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[])
            { {
                      .uuid = &gatt_svr_chr_model_number_uuid.u,
                      .access_cb = gatt_svr_chr_access_all,
                      .flags = BLE_GATT_CHR_F_READ,
              },
              {
                      .uuid = &gatt_svr_chr_serial_number_uuid.u,
                      .access_cb = gatt_svr_chr_access_all,
                      .flags = BLE_GATT_CHR_F_READ,
              },
              {
                      .uuid = &gatt_svr_chr_firmware_revision_uuid.u,
                      .access_cb = gatt_svr_chr_access_all,
                      .flags = BLE_GATT_CHR_F_READ,
              },
              {
                      .uuid = &gatt_svr_chr_manufacturer_name_uuid.u,
                      .access_cb = gatt_svr_chr_access_all,
                      .flags = BLE_GATT_CHR_F_READ,
              },
              {
                      0, /* No more characteristics in this service. */
              }
            },
    },
    {
     .type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &gatt_svr_svc_custom_temperature_service_uuid.u,
     .characteristics = (struct ble_gatt_chr_def[])
         { {
               .uuid = &gatt_svr_chr_temperature_stream_uuid.u,
               .access_cb = gatt_svr_chr_access_all,
               .val_handle = &temperature_notify_handle,
               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
           },
           {
               .uuid = &gatt_svr_chr_temperature_memory_status_uuid.u,
               .access_cb = gatt_svr_chr_access_all,
               .flags = BLE_GATT_CHR_F_INDICATE,
           },
           {
               0, /* No more characteristics in this service. */
           }
         },
    },
    {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &gatt_svr_chr_custom_humidity_service_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[])
        { {
                  .uuid = &gatt_svr_chr_humidity_stream_uuid.u,
                  .access_cb = gatt_svr_chr_access_all,
                  .val_handle = &humidity_notify_handle,
                  .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          },
          {
                  .uuid = &gatt_svr_chr_humidity_memory_status_uuid.u,
                  .access_cb = gatt_svr_chr_access_all,
                  .flags = BLE_GATT_CHR_F_INDICATE,
          },
          {
                  0, /* No more characteristics in this service. */
          }
        },
    },
    {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &gatt_svr_chr_custom_pressure_service_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[])
        { {
                  .uuid = &gatt_svr_chr__pressure_stream_uuid.u,
                  .access_cb = gatt_svr_chr_access_all,
                  .val_handle = &pressure_notify_handle,
                  .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          },
          {
                  .uuid = &gatt_svr_chr_pressure_memory_status_uuid.u,
                  .access_cb = gatt_svr_chr_access_all,
                  .flags = BLE_GATT_CHR_F_INDICATE,
          },
          {
                  0, /* No more characteristics in this service. */
          }
        },
    },
    {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &gatt_svr_chr_custom_microphone_service_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[])
        { {
                  .uuid = &gatt_svr_chr_microphone_stream_uuid.u,
                  .access_cb = gatt_svr_chr_access_all,
                  .val_handle = &audio_notify_handle,
                  .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          },
          {
                  .uuid = &gatt_svr_chr_microphone_memory_status_uuid.u,
                  .access_cb = gatt_svr_chr_access_all,
                  .flags = BLE_GATT_CHR_F_INDICATE,
          },
          {
                  0, /* No more characteristics in this service. */
          }
        },
    },
    {
     0, /* No more services. */
    },
};

/* Private function definitions --------------------------------------------------------------------------------------*/
static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len) {

    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);

    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);

    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;

}

static int gatt_svr_chr_access_all(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt,
                                   void *arg) {

    const ble_uuid_t *uuid;
    int rc;
    char write_response_table[512];

    uuid = ctxt->chr->uuid;

    /* Determine which characteristic is being accessed by examining its
     * 128-bit UUID.
     */

    if (ble_uuid_cmp(uuid, &gatt_svr_chr_temperature_stream_uuid.u) == 0) {
        switch (ctxt->op) {
            case BLE_GATT_ACCESS_OP_READ_CHR:

                ESP_LOGI(TAG, "Temperature Stream characteristic: read requested");

                //TODO: Replace with real data
                char *temperatureStream = "abc";

                rc = os_mbuf_append(ctxt->om, temperatureStream, strlen(temperatureStream));

                //free(temperatureStream);

                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
        }


    } else if (ble_uuid_cmp(uuid, &gatt_svr_chr_model_number_uuid.u) == 0) {
        switch (ctxt->op) {
            case BLE_GATT_ACCESS_OP_READ_CHR:

                ESP_LOGI(TAG, "Device Model Number characteristic: read requested");

                rc = os_mbuf_append(ctxt->om, DEVICE_MODEL_NUMBER, strlen(DEVICE_MODEL_NUMBER));

                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
        }
    } else if (ble_uuid_cmp(uuid, &gatt_svr_chr_serial_number_uuid.u) == 0) {
        switch (ctxt->op) {
            case BLE_GATT_ACCESS_OP_READ_CHR:

                ESP_LOGI(TAG, "Device Serial Number characteristic: read requested");

                rc = os_mbuf_append(ctxt->om, DEVICE_SERIAL_NUMBER, strlen(DEVICE_SERIAL_NUMBER));

                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
        }
    } else if (ble_uuid_cmp(uuid, &gatt_svr_chr_firmware_revision_uuid.u) == 0) {
        switch (ctxt->op) {
            case BLE_GATT_ACCESS_OP_READ_CHR:

                ESP_LOGI(TAG, "Device Firmware Revision characteristic: read requested");

                rc = os_mbuf_append(ctxt->om, DEVICE_FIRMWARE_REVISION, strlen(DEVICE_FIRMWARE_REVISION));

                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
        }
    } else if (ble_uuid_cmp(uuid, &gatt_svr_chr_manufacturer_name_uuid.u) == 0) {
        switch (ctxt->op) {
            case BLE_GATT_ACCESS_OP_READ_CHR:

                ESP_LOGI(TAG, "Device Manufacturer Name characteristic: read requested");

                rc = os_mbuf_append(ctxt->om, DEVICE_MANUFACTURER_NAME, strlen(DEVICE_MANUFACTURER_NAME));

                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
        }
    } else if (ble_uuid_cmp(uuid, &gatt_svr_chr_current_time_uuid.u) == 0) {
        switch (ctxt->op) {
            case BLE_GATT_ACCESS_OP_READ_CHR:

                ESP_LOGI(TAG, "Current Time characteristic: read requested");

                uint8_t * currentTime = get_time();

                rc = os_mbuf_append(ctxt->om, currentTime, 10);

                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

            case BLE_GATT_ACCESS_OP_WRITE_CHR:

                ESP_LOGI(TAG, "Current Time characteristic: write requested");

                rc = gatt_svr_chr_write(ctxt->om, 1, sizeof write_response_table, write_response_table, NULL);

                set_time(ctxt->om->om_data);

                return rc;
            default:
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
        }
    }

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;

}
/* Exported function definitions -------------------------------------------------------------------------------------*/
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {

    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            MODLOG_DFLT(DEBUG, "Registered service %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                        ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            MODLOG_DFLT(DEBUG, "Registering characteristic %s with "
                               "def_handle=%d val_handle=%d\n",
                        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                        ctxt->chr.def_handle,
                        ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            MODLOG_DFLT(DEBUG, "Registering descriptor %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                        ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
    }

}

int gatt_svr_init(void) {

    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    // Allocate initially with 0 value holders for BLE characteristics payloads
    gatt_svr_chr_temperature_stream_value = (uint8_t *)calloc(1, sizeof(uint8_t));
    gatt_svr_chr_humidity_stream_value = (uint8_t *)calloc(1, sizeof(uint8_t));
    gatt_svr_chr_pressure_stream_value = (uint8_t *)calloc(1, sizeof(uint8_t));
    gatt_svr_chr_audio_stream_value = (uint8_t *)calloc(1, sizeof(uint8_t));

    return 0;

}

void send_temperature_notification(void) {
    if (temperature_notification_enabled == 1) {
        int rc = 0;

        struct os_mbuf *om;

        // TODO: Replace with real data
        char *temperatureStream = "abcd";

        om = ble_hs_mbuf_from_flat(temperatureStream, 4);
        rc = ble_gattc_notify_custom(conn_handle, temperature_notify_handle, om);

        if (rc != 0) {
            ESP_LOGE(TAG, "Error while sending temperature stream notification; rc = %d", rc);
        } else {
            ESP_LOGI(TAG, "Sent temperature stream notification");
        }

    }
}

void send_humidity_notification(void) {
    if (humidity_notification_enabled == 1) {
        int rc = 0;

        struct os_mbuf *om;

        // TODO: Replace with real data
        char *humidityStream = "deif";

        om = ble_hs_mbuf_from_flat(humidityStream, 4);
        rc = ble_gattc_notify_custom(conn_handle, humidity_notify_handle, om);

        if (rc != 0) {
            ESP_LOGE(TAG, "Error while sending humidity stream notification; rc = %d", rc);
        } else {
            ESP_LOGI(TAG, "Sent humidity stream notification");
        }

    }
}

void send_pressure_notification(void) {
    if (pressure_notification_enabled == 1) {
        int rc = 0;

        struct os_mbuf *om;

        // TODO: Replace with real data
        char *pressureStream = "ghii";

        om = ble_hs_mbuf_from_flat(pressureStream, 4);
        rc = ble_gattc_notify_custom(conn_handle, pressure_notify_handle, om);

        if (rc != 0) {
            ESP_LOGE(TAG, "Error while sending pressure stream notification; rc = %d", rc);
        } else {
            ESP_LOGI(TAG, "Sent pressure stream notification");
        }

    }
}

void send_audio_notification(void) {
    if (audio_notification_enabled == 1) {
        int rc = 0;

        struct os_mbuf *om;

        // TODO: Replace with real data
        char *audioStream = "jkll";

        om = ble_hs_mbuf_from_flat(audioStream, 4);
        rc = ble_gattc_notify_custom(conn_handle, audio_notify_handle, om);

        if (rc != 0) {
            ESP_LOGE(TAG, "Error while sending audio stream notification; rc = %d", rc);
        } else {
            ESP_LOGI(TAG, "Sent audio stream notification");
        }

    }
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
