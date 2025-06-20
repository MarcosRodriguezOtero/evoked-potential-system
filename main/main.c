#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stdlib.h>
#include <math.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hal/adc_types.h"
#include "driver/i2c_master.h"   // para i2c_new_master_bus() y i2c_master_transmit()
#include "esp_log.h"             


#define ADC_CHANNEL ADC_CHANNEL_4   // Canal ADC4 (A4, consulta el mapeo en tu placa)
#define ADC_WIDTH ADC_WIDTH_BIT_12  // Resolución de 12 bits (0-4095)
#define ADC_ATTEN ADC_ATTEN_DB_11   // Atenuación 11 dB (rango 0-3.3V)
#define DEFAULT_VREF 1100           // Valor de referencia de voltaje (mV)
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
static esp_adc_cal_characteristics_t *adc_chars;
float numeros1[1000];  // Almacén para los valores convertidos
float numeros2[1000];
double t[1000];       // Tiempo para cada muestra

    
// Función para inicializar el ADC
void init_adc() { 
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars);
}

// Tarea para leer y muestrear la señal
void adc_sampling_task(void *pvParameters) {
    const TickType_t delay = pdMS_TO_TICKS(1000 / SAMPLING_FREQUENCY);  // Intervalo entre muestras

for (int i = 0; i < 100; i++) {
  //int i = 0;
 //while (true) {
        // Leer el valor crudo del ADC
        uint32_t adc_raw = adc1_get_raw(ADC_CHANNEL);

        // Convertir a milivoltios
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_raw, adc_chars);

        // Guardar en los arrays para el algoritmo
        numeros1[i] = (float)voltage;  // Guardar en numeros1 en mV
        numeros2[i] = (float)voltage;  // Guardar en numeros2 en mV

        // Imprimir el valor crudo y el voltaje (solo para depuración)
        printf("ADC Raw: %" PRIu32 "\tVoltage: %" PRIu32 "mV\n", adc_raw, voltage);

        // Esperar el siguiente muestreo
        vTaskDelay(delay);
        
      // i++;
    }
}


typedef struct {
    double valor;
    double tiempo;
} Pico;
typedef struct {
    int num_picos;
    Pico *picos;
} ResultadoPicos;

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
void changeSign(double *vector) {
    ESP_LOGI(TAG, "signo cambiado");
     if (vector == NULL) {
        printf("el archivo esta vacio.");
        return;
    }
    //int size = sizeof(vector) / sizeof(vector[0]);
    for (int i = 0; i < 1000; i++) {
        vector[i] = -vector[i];
    }
    printf("signo cambiado");
} 
ResultadoPicos encontrar_picos(double* datos, double* tiempos, int tamano) {
    ResultadoPicos resultado;
    resultado.num_picos = 0;
    resultado.picos = (Pico*)malloc(tamano * sizeof(Pico));
    if (resultado.picos == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria para el array de picos\n");
        resultado.num_picos = -1;
        return resultado;
    }
    int r;
    for (int i = 1; i < tamano - 1; i++) {

    	//int l=1; 
    	r=1;
    	//while(i - l >= 0 && datos[i - l] == datos[i]){
    		//l++;
    	//}
    	while(i + r < tamano && datos[i + r] == datos[i]){
    	    r++;
    	    	}
        if ((i + r < tamano) && datos[i] > datos[i - 1] && datos[i] > datos[i + r]) {
            //printf("pico: %i %i %.11f %.11f %.11f ", num_picos,i,datos[i], datos[i-1], datos[i+1]);
            resultado.picos[resultado.num_picos].valor = datos[i];
            resultado.picos[resultado.num_picos].tiempo = tiempos[i];
            resultado.num_picos++;
        }
        i=i + r - 1;
    }
    resultado.picos = (Pico*)realloc(resultado.picos, resultado.num_picos * sizeof(Pico));
    if (resultado.picos == NULL && resultado.num_picos > 0) {
        fprintf(stderr, "Error: no se pudo redimensionar el array de picos\n");
        resultado.num_picos = -1;
    }
    return resultado;
}
int comparar_picos(const void *a, const void *b) {
    Pico *picoA = (Pico *)a;
    Pico *picoB = (Pico *)b;

    if (campo == 1) {
        if (picoA->valor < picoB->valor) return 1;
        if (picoA->valor > picoB->valor) return -1;
    } else if (campo == 0) {
        if (picoA->tiempo < picoB->tiempo) return 1;
        if (picoA->tiempo > picoB->tiempo) return -1;
    }
return 0;
}
Pico* detectarP25(double *signal,double *time,int tamano){
   // changeSign(signal);
    ResultadoPicos resultado = encontrar_picos(signal,time,tamano);
    if (resultado.picos == NULL) {
            return NULL;
        }
    Pico *filtered_picos = (Pico *)malloc(resultado.num_picos * sizeof(Pico));
    if (filtered_picos == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria para los picos filtrados\n");
        free(resultado.picos);
        return NULL;
    }
    int filtered_count = 0;
    for (int i = 0; i < resultado.num_picos; i++) {
        if (resultado.picos[i].tiempo >= 19.5 && resultado.picos[i].tiempo <= 32.0) {
            filtered_picos[filtered_count++] = resultado.picos[i];
        }
    }
    Pico *picos_selec = NULL;
    if (filtered_count > 0) {
    	campo=1;
        qsort(filtered_picos, filtered_count, sizeof(Pico), comparar_picos);
        picos_selec = (Pico *)malloc(3 * sizeof(Pico));
        if (picos_selec == NULL) {
            fprintf(stderr, "Error: no se pudo reservar memoria para los picos seleccionados\n");
            free(filtered_picos);
            free(resultado.picos);
            return NULL;
        }

    printf("Se ha detectado la onda P25. Las latencias y amplitudes son:\n");
    for (int i = 0; i < 3 && i < filtered_count; i++) {
            picos_selec[i]=filtered_picos[i];
            Pico b = filtered_picos[i];
            cuentaP25++;
            printf("Latencia: %.4f, Amplitud: %.7f\n", picos_selec[i].tiempo, picos_selec[i].valor);
    }
    } else {
        printf("No se ha detectado la onda P25\n");
    }

    free(filtered_picos);
    free(resultado.picos);
    return picos_selec;
}

Pico* detectarN20(double *signal,double *time,int tamano,Pico* picosP25) {
    changeSign(signal);
    ResultadoPicos resultado = encontrar_picos(signal,time,tamano);
    if (resultado.picos == NULL) {
          return NULL;
      }
    Pico *filtered_picos = (Pico *)malloc(resultado.num_picos * sizeof(Pico));
    if (filtered_picos == NULL) {
        fprintf(stderr, "Error: no se pudo reservar memoria para los picos filtrados\n");
        free(resultado.picos);
        return NULL;
    }
    int filtered_count = 0;

    for (int i = 0; i < resultado.num_picos; i++) {
    	if (resultado.picos[i].tiempo >= 15.5 && resultado.picos[i].tiempo <= 29.0) {
    		filtered_picos[filtered_count++] = resultado.picos[i];
    		Pico a= resultado.picos[i];
            printf("Pico %d: Valor = %.7f, Tiempo = %.7f\n", i + 1, resultado.picos[i].valor, resultado.picos[i].tiempo);
        }
    }


    if (filtered_count > 0) {
    	campo=1;
        qsort(filtered_picos, filtered_count, sizeof(Pico), comparar_picos);
    }
    if (cuentaP25 > 0) {
    	campo=0;
                qsort(picosP25, cuentaP25, sizeof(Pico), comparar_picos);
            }
        Pico maxN20=filtered_picos[0];
        Pico maxP25= picosP25[0];
        for (int i = 0; i < filtered_count; i++) {
        		Pico a= filtered_picos[i];
        		printf("Pico  Valor = 7f, Tiempo =f\n");

        }
        for (int i = 0; i < filtered_count; i++) {
        	double r = filtered_picos[i].tiempo;
            if (filtered_picos[i].tiempo < maxP25.tiempo){
            maxN20=filtered_picos[i];
            break;
            }
        }

    Pico *picosP25_selec= (Pico *)malloc(3 * sizeof(Pico));
    if (picosP25_selec == NULL) {
            fprintf(stderr, "Error: no se pudo reservar memoria para los picos seleccionados\n");
            free(filtered_picos);
            free(resultado.picos);
            return NULL;
        }
    int num=0;
    for (int i = 0; i < cuentaP25; i++) {
        if (picosP25[i].tiempo > (maxN20.tiempo + 0.5)){
        picosP25_selec[num++]=picosP25[i];

        }
    }
    int indice=0;
    double minimo=picosP25_selec[0].tiempo- maxN20.tiempo;
     for (int i = 1; i < num; i++){
    	 double diff = picosP25_selec[i].tiempo - maxN20.tiempo;
        if (diff < minimo) {
            minimo=diff;
            indice=i;
        }
     }

    Pico P25=picosP25_selec[indice];
    maxN20.valor = maxN20.valor + P25.valor;
    Pico *res= (Pico *)malloc(3 * sizeof(Pico));
    if (res == NULL) {
            fprintf(stderr, "Error: no se pudo reservar memoria para los picos seleccionados\n");
            free(filtered_picos);
            free(picosP25_selec);
            free(resultado.picos);
            return NULL;
        }
    res[0]=P25;
    res[1]=maxN20;


    free(filtered_picos);
    free(picosP25_selec);
    free(resultado.picos);

     if ((maxN20.tiempo <=23) && (maxN20.tiempo >= 16) && (maxN20.valor >=0.6)){
        printf("Se ha detectado la onda N20. La latencia es: %.4f y la amplitud es: %.7f\n", maxN20.tiempo,maxN20.valor);
        res[2].valor=1;
        return res;
     }
    else {
        printf("No se ha detectado la onda N20");
        res[2].valor=0;
        return res;

    }
}

int algoritmo(void) {


    //changeSign(numeros1);
    //changeSign(numeros2);
    Pico* picos_detec1= detectarP25(numeros1, t, 1000);
    if (picos_detec1 == NULL) {
        printf("No se detectaron picos.\n");
        return 2;
    }
/*
    double valor=0;
    double tiempo=0;
    for (int i=0; i<3;i++){
    	valor=picos_detec1[i].valor;
    	tiempo=picos_detec1[i].tiempo;
    }*/
    Pico* detec1N20=detectarN20(numeros1,t,1000,picos_detec1);
	free(picos_detec1);
	if (detec1N20 == NULL) {
		printf("No se detectaron picos.\n");
		return 3;
	}

	cuentaP25=0;
    Pico* picos_detec2= detectarP25(numeros2, t, 1000);
    if (picos_detec2 == NULL) {
        printf("No se detectaron picos.\n");
        free(detec1N20);
        return 2;
    } 

    Pico* detec2N20=detectarN20(numeros2,t,1000,picos_detec2);
    free(picos_detec2);

     if (detec2N20 == NULL) {
        printf("No se detectaron picos.\n");
        free(detec1N20);
        return 3;
    }

    if (detec1N20[2].valor != detec2N20[2].valor) {
        printf("Prueba inconclusa, por favor vuelva a repetir la prueba\n");
        free(detec1N20);
        free(detec2N20);
        return 4;
    } else if (detec1N20[2].valor == 1) {
        printf("Se ha detectado la onda N20.\n");
        free(detec1N20);
        free(detec2N20);
        return 5;
    } else {
        printf("No se ha detectado la onda N20\n");
        free(detec1N20);
        free(detec2N20);
        return 6;
    }


    return 0;
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