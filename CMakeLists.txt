cmake_minimum_required(VERSION 3.28.0)

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/esp32s3.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(PROJET_GATE_WAY)

target_sources(app PRIVATE
    main.c
    components/wifi/src/wifi.c
    components/bacnet/src/bacnet.c
    components/modem/src/modem.c
)

target_include_directories(app PRIVATE
  components/wifi/include
  components/bacnet/include
  components/modem/include
)
