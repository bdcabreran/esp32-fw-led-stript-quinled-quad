
/* RMT example -- RGB LED Strip
   This example uses the RMT peripheral to control a RGB LED strip.
   Author: Bayron Cabrera
   Email: bayron.nanez@gmail.com
*/

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "led_strip.h"

static const char *TAG = "led_strip_example";


// LED1 Strip configuration
#define LED1_RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED1_RMT_TX_GPIO (16)
#define LED1_STRIP_LED_NUMBER (30)
#define LED1_CHASE_SPEED_MS (25)

// LED2 Strip configuration
#define LED2_RMT_TX_CHANNEL RMT_CHANNEL_1
#define LED2_RMT_TX_GPIO (3)
#define LED2_STRIP_LED_NUMBER (30)
#define LED2_CHASE_SPEED_MS (50)

// LED3 Strip configuration
#define LED3_RMT_TX_CHANNEL RMT_CHANNEL_2
#define LED3_RMT_TX_GPIO (1)
#define LED3_STRIP_LED_NUMBER (30)
#define LED3_CHASE_SPEED_MS (50)

// LED4 Strip configuration
#define LED4_RMT_TX_CHANNEL RMT_CHANNEL_3
#define LED4_RMT_TX_GPIO (4)
#define LED4_STRIP_LED_NUMBER (30)
#define LED4_CHASE_SPEED_MS (100)


/**
 * @brief Print the startup message to the monitor
 *
 */
void print_startup_message() {
    const char* projectName = "esp32-fw-led-strip-quinled-quad";
    const char* author = "Bayron Cabrera";
    const char* date = "2023-01-03";
    const char* version = "1.0.0";

    printf("Project: %s\n", projectName);
    printf("Author: %s\n", author);
    printf("Date: %s\n", date);
    printf("Version: %s\n", version);
}



/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

/**
 * @brief Simple helper function, to create a rainbow chase effect
 *
 */
void led_rainbow_chase(led_strip_t *strip, uint16_t start_rgb, uint16_t chase_speed_ms, uint16_t led_number) {
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = i; j < led_number; j += 3) {
            // Build RGB values
            hue = j * 360 / led_number + start_rgb;
            led_strip_hsv2rgb(hue, 100, 20, &red, &green, &blue);
            // Write RGB values to strip driver
            ESP_ERROR_CHECK(strip->set_pixel(strip, j, red, green, blue));
        }
        // Flush RGB values to LEDs
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
        vTaskDelay(pdMS_TO_TICKS(chase_speed_ms));
        strip->clear(strip, 50);
        vTaskDelay(pdMS_TO_TICKS(chase_speed_ms));
    }
    start_rgb += 60;
}

/**
 * @brief Task to create a rainbow chase effect on LED1 strip
 *
 */
void led_rainbow_chase_task1(void *pvParameters) {

    led_strip_t *strip1 =  led_strip_init(LED1_RMT_TX_CHANNEL, LED1_RMT_TX_GPIO, LED1_STRIP_LED_NUMBER, LED_STRIP_WS2812);
    if (!strip1) {
        ESP_LOGE("main", "LED strip1 initialization failed");
        return;
    }

    uint16_t start_rgb = 0;
    ESP_LOGI(TAG, "LED1 Rainbow Chase Start");

    while (true) {
        led_rainbow_chase(strip1, start_rgb, LED1_CHASE_SPEED_MS, LED1_STRIP_LED_NUMBER);
    }
}

/**
 * @brief Task to create a rainbow chase effect on LED2 strip
 *
 */
void led_rainbow_chase_task2(void *pvParameters) {

    led_strip_t *strip4 =  led_strip_init(LED4_RMT_TX_CHANNEL, LED4_RMT_TX_GPIO, LED4_STRIP_LED_NUMBER, LED_STRIP_WS2812);
    if (!strip4) {
        ESP_LOGE("main", "LED strip4 initialization failed");
        return;
    }

    uint16_t start_rgb = 0;
    ESP_LOGI(TAG, "LED4 Rainbow Chase Start");

    while (true) {
        led_rainbow_chase(strip4, start_rgb, LED4_CHASE_SPEED_MS, LED4_STRIP_LED_NUMBER);
    }
}


/**
 * @brief Main function
 *
 */
void app_main(void)
{
    print_startup_message();
    xTaskCreate(led_rainbow_chase_task1, "LED Rainbow Chase Task 1", 2048, NULL, 5, NULL);
    xTaskCreate(led_rainbow_chase_task2, "LED Rainbow Chase Task 2", 2048, NULL, 5, NULL);

}



