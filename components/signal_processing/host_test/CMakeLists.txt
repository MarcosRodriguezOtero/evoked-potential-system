cmake_minimum_required(VERSION 3.16)

set(COMPONENTS main)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "$ENV{IDF_PATH}/tools/mocks/freertos/"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(signal_processing_host_test)
