#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stdlib.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/i2c_master.h"   // para i2c_new_master_bus() y i2c_master_transmit()
#include "esp_log.h"             
#include "signal_processing.h"

#define ADC_CHANNEL ADC_CHANNEL_4   // Canal ADC4 (A4, consulta el mapeo en tu placa)
#define ADC_WIDTH ADC_BITWIDTH_12
#define ADC_ATTEN ADC_ATTEN_DB_12
#define SAMPLING_FREQUENCY 1000     // Frecuencia de muestreo (Hz)

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define DAC_ADDR 0x60
#define I2C_FREQ_HZ 100000  // 100kHz
 
 //Funcion para inicializar el DAC
 
static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t dac_handle = NULL;

void stim_i2c_init(void) {
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    i2c_new_master_bus(&bus_config, &i2c_bus);

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DAC_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    i2c_master_bus_add_device(i2c_bus, &dev_config, &dac_handle);
}

void stim_send_to_dac(uint16_t value) {
    uint8_t data[2];
    data[0] = (value >> 4) & 0xFF;
    data[1] = (value & 0x0F) << 4;
    i2c_master_transmit(dac_handle, data, 2, 100);
}
 
 
// Variables globales
static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;
static bool adc_calibrated = false;
double numeros1[1000];  // Almacén para los valores convertidos
double numeros2[1000];
double t[1000];       // Tiempo para cada muestra

    
// Función para inicializar el ADC
void init_adc(void) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_WIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &channel_config));

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_WIDTH,
    };
    adc_calibrated = (adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle) == ESP_OK);
}

// Tarea para leer y muestrear la señal
void adc_sampling_task(void *pvParameters) {
    const TickType_t delay = pdMS_TO_TICKS(1000 / SAMPLING_FREQUENCY);  // Intervalo entre muestras

for (int i = 0; i < 100; i++) {
  //int i = 0;
 //while (true) {
        // Leer el valor crudo del ADC
        int adc_raw = 0;
        int voltage = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw));

        // Convertir a milivoltios
        if (adc_calibrated) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage));
        } else {
            voltage = adc_raw;
        }

        // Guardar en los arrays para el algoritmo
        numeros1[i] = (double)voltage;  // Guardar en numeros1 en mV
        numeros2[i] = (double)voltage;  // Guardar en numeros2 en mV

        // Imprimir el valor crudo y el voltaje (solo para depuración)
        printf("ADC Raw: %d\tVoltage: %dmV\n", adc_raw, voltage);

        // Esperar el siguiente muestreo
        vTaskDelay(delay);
        
      // i++;
    }
}


static const char *TAG = "example";
int cuentaP25=0;
int campo= 0;


#define BLINK_GPIO 13

//static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}



#else
#endif


static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


void app_main(void) {
		
	 init_adc();  
	 printf("Iniciando captura de datos...\n");
	 
	 // Capturar la señal del ADC antes de ejecutar el algoritmo
    adc_sampling_task(NULL);
    
    stim_i2c_init();            // Inicia el bus I2C
    stim_send_to_dac(4095);     // Enviar corriente (por ejemplo, 2048 / 4095)
    
    xTaskCreate(adc_sampling_task, "adc_task", 4096, NULL, 5, NULL);
	 
	 int result = algoritmo();
	 // Sí N20
	 if (result==5){

         configure_led();

         while (1) {

     		gpio_set_level(BLINK_GPIO, 1);
     		//vTaskDelay(25);
     		//gpio_set_level(BLINK_GPIO, 0);
     		//vTaskDelay(25);
         }
	 }
	 // Inconclusa
	 else if (result==4){
         configure_led();
		 while (1) {
	      gpio_set_level(BLINK_GPIO, 1);
	      vTaskDelay(25);
	      gpio_set_level(BLINK_GPIO, 0);
	      vTaskDelay(25);
	 }
	 }
	 // No N20
	 else if (result ==6){

			 configure_led();

		 while (1) {
				gpio_set_level(BLINK_GPIO, 1);
				vTaskDelay(100);
				gpio_set_level(BLINK_GPIO, 0);
				vTaskDelay(100);
			 }
			 //errores
		 }else{
			 configure_led();

						 while (1) {
							gpio_set_level(BLINK_GPIO, 1);
							vTaskDelay(200);
							gpio_set_level(BLINK_GPIO, 0);
							vTaskDelay(200);

	 }
		 }

}
