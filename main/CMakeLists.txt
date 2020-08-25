set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
idf_component_register(SRCS
    "paf.c"
    "paf_console.c"
    "paf_flash.c"
    "paf_led.c"
    "paf_util.c"
    "paf_webserver.c"
    "paf_wifi.c"
    INCLUDE_DIRS ${PROJECT_SOURCE_DIR})

