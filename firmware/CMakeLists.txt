cmake_minimum_required(VERSION 3.20)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(vr_trackers)

target_sources(app PRIVATE src/main.c)