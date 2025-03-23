/**
  **********************************************************************************************************************
  * @file    i2c_interface.c
  * @brief   This file is the I2C API implementation
  * @authors patrykmonarcha
  * @date Dec 3, 2024
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include "i2c_interface.h"
#include "esp_log.h"
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/
#define I2C_PORT_AUTO (-1)

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "i2c_interface";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/

/* Private function definitions --------------------------------------------------------------------------------------*/

/* Exported function definitions -------------------------------------------------------------------------------------*/
i2c_master_bus_handle_t initializeI2CBus(uint8_t sda_pin, uint8_t scl_pin) {
    i2c_master_bus_config_t i2c_bus_config = {
            .i2c_port = I2C_PORT_AUTO,
            .sda_io_num = sda_pin,
            .scl_io_num = scl_pin,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;

    esp_err_t ret = i2c_new_master_bus(&i2c_bus_config, &bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
    }
    ESP_LOGD(TAG,"I2C master bus created");

    return bus_handle;
}


/* END OF FILE -------------------------------------------------------------------------------------------------------*/
