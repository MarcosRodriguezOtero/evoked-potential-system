# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        signal_processing
        esp_adc
        esp_driver_gpio
        esp_driver_i2c
)
