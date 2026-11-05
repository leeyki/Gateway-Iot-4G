cmake_minimum_required(VERSION 3.28.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_wifi)

target_sources(app PRIVATE
    main.c
    components/wifi/src/wifi.c
    components/bacnet/src/bacnet.c
)

target_include_directories(app PRIVATE
  components/wifi/include
  components/bacnet/include
)
