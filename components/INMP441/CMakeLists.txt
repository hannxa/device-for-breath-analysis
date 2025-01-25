idf_component_register(SRCS
        "inmp441_driver.c"
        "inmp441_app.c"
        INCLUDE_DIRS "include"
        "../i2s_interface/include"
        REQUIRES driver
        INMP441)