#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/adc.h"


// #define IN1 GPIO_NUM_38
// #define EN1 GPIO_NUM_39
// #define IN2 GPIO_NUM_40
// #define EN2 GPIO_NUM_41

#define IN1    38
#define IN2    40
#define EN1    39
#define EN2    41
#define nSLEEP 45
#define GPIO_OUTPUT_PIN_SEL  ( (1ULL<<IN1) | (1ULL<<IN2) | (1ULL<<EN1) | (1ULL<<EN2) |  (1ULL<<nSLEEP)  )


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (8) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (0) // 8192 for 100%. To be decided.
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#define DEFAULT_VREF    1100  // Reference voltage in mV (adjust based on your board calibration)
#define ADC_WIDTH       ADC_WIDTH_BIT_13  // 12-bit resolution
#define ADC_ATTEN       ADC_ATTEN_DB_11   // 11dB attenuation (for higher voltage range)
#define ADC_CHANNEL     ADC1_CHANNEL_8    // GPIO14 is ADC1 Channel 6 on ESP32-S2

int strength_current = 4095;

int STIMStrength[] = {0, 819, 1638, 2457, 3276, 4095, 4914, 5733, 6552, 7371, 8190};

static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = LEDC_DUTY, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void get_battery_voltage() {
    // Set up ADC configuration
    // adc2_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    adc1_config_width(ADC_WIDTH);
    // Read raw ADC value
    int raw_out=0;
    int adc_reading = adc1_get_raw(ADC_CHANNEL);
    printf("Raw Batt: %d\n",adc_reading);
    if(adc_reading<1000)
    {
        strength_current = 0;
    }
}


void app_main(void)
{   
    // gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
    // gpio_set_direction(nSLEEP, GPIO_MODE_OUTPUT);
    // gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
    // gpio_set_direction(EN1, GPIO_MODE_OUTPUT);
    // gpio_set_direction(EN2, GPIO_MODE_OUTPUT);
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    example_ledc_init();
    gpio_set_level(nSLEEP, 1);
    gpio_set_level(EN1, 1);
    gpio_set_level(EN2, 1);
    // gpio_set_level(IN1, 1);
    // gpio_set_level(IN2, 1);

    // gpio_set_leve(IN1, 1);
    // gpio_set_level(IN2, 1);
    while(1)
    {
            
            printf("Driver on, strength set : %d \n", strength_current);
            for(int i= 0;i<20;i++)
            {
                
                /*gpio_set_level(IN1, 1);
                gpio_set_level(IN2, 0);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                gpio_set_level(IN1, 0);
                gpio_set_level(IN2, 1);
                vTaskDelay(1000 / portTICK_PERIOD_MS);*/
                
                gpio_set_level(IN1, 1);
                gpio_set_level(IN2, 0);
                if(i==5)
                {
                    get_battery_voltage();
                }
                
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, strength_current));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
                vTaskDelay(10 / portTICK_PERIOD_MS);
                gpio_set_level(IN1, 0);
                gpio_set_level(IN2, 1);
                vTaskDelay(10 / portTICK_PERIOD_MS);

                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

                gpio_set_level(IN1, 0);
                gpio_set_level(IN2, 0);
                vTaskDelay(12.5 / portTICK_PERIOD_MS);
                
                // ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, strength_current));
                // ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            }
            

            //gpio_set_level(IN1, 1);
            //gpio_set_level(IN2, 0);
            

            printf("Stimulation fired : driver off for 2 sec\n");
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}