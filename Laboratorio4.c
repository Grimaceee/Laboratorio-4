#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "esp_system.h"

#define SERVO_PIN       14
#define ROJO_PIN        12
#define VERDE_PIN       13
#define AZUL_PIN        27
#define BOTON1_PIN      4
#define BOTON2_PIN      5
#define BOTON3_PIN      18
#define BOTON4_PIN      19

#define SERVO_MIN       40
#define SERVO_MAX       115

int posicion_servo = 77;
int indice_color = 0;
int brillo = 0;
int modo = 0;

void mover_servo(int posicion) {
    posicion = (posicion < SERVO_MIN) ? SERVO_MIN : (posicion > SERVO_MAX) ? SERVO_MAX : posicion;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, posicion);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void ajustar_brillo(ledc_channel_t canal, int brillo) {
    int ciclo_trabajo = brillo * 1023 / 100;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, canal, ciclo_trabajo);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, canal);
}

int mapear_rango(int x, int min_in, int max_in, int min_out, int max_out) {
    return (x - min_in) * (max_out - min_out) / (max_in - min_in) + min_out;
}

void app_main() {
    ledc_timer_config_t servo_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .freq_hz          = 50,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&servo_timer);

    ledc_channel_config_t servo_channel = {
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_PIN,
        .duty           = posicion_servo,
        .hpoint         = 0
    };
    ledc_channel_config(&servo_channel);

    ledc_timer_config_t led_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_1,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&led_timer);

    ledc_channel_config_t rojo_channel = {
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = ROJO_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&rojo_channel);

    ledc_channel_config_t verde_channel = {
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_2,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = VERDE_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&verde_channel);

    ledc_channel_config_t azul_channel = {
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_3,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = AZUL_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&azul_channel);

    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BOTON1_PIN) | (1ULL << BOTON2_PIN) | (1ULL << BOTON3_PIN) | (1ULL << BOTON4_PIN),
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

    while (1) {
        if (!gpio_get_level(BOTON1_PIN)) {
            posicion_servo++;
            mover_servo(posicion_servo);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if (!gpio_get_level(BOTON2_PIN)) {
            posicion_servo--;
            mover_servo(posicion_servo);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if (!gpio_get_level(BOTON3_PIN)) {
            indice_color = (indice_color + 1) % 4;
            modo = (indice_color == 3) ? 1 : 0;
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (!gpio_get_level(BOTON4_PIN) && modo == 0) {
            brillo += 10;
            if (brillo > 100) brillo = 0;
            ajustar_brillo((ledc_channel_t)indice_color, brillo);
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        if (modo == 1) {
            int posicion_adc = adc1_get_raw(ADC1_CHANNEL_0);
            int valor_rojo = mapear_rango(posicion_adc, 0, 4095, 0, 1023);
            int valor_verde = mapear_rango(posicion_adc, 0, 4095, 1023, 0);
            int valor_azul = mapear_rango(posicion_adc, 0, 4095, 0, 1023);

            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, valor_rojo);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, valor_verde);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_3, valor_azul);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_3);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
