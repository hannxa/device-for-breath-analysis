idf_component_register(SRCS
        "ble_gap.c"
        "ble_gatt.c"
        INCLUDE_DIRS "include"
        REQUIRES bt
                 nvs_flash
                 rtc_driver
                 data_storage
        )