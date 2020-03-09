
#include "Arduino.h"
#include <OLEDDisplayUi.h>

static const char *TAG = "main";

extern OLEDDisplayUi ui;
 
extern "C" void initSNTP();
extern void setupSSD1306();
extern void dimSSD1306();

void blinkTask(void *pvParameter) {
    //GPIO_NUM_16 is G16 on board
    gpio_set_direction(GPIO_NUM_16,GPIO_MODE_OUTPUT);
    printf("Blinking LED on GPIO 16\n");
    int cnt=0;
    while (1) {
		gpio_set_level(GPIO_NUM_16,cnt%2);
        cnt++;
        vTaskDelay(500/portTICK_PERIOD_MS);
        }   
}

extern "C" 
void app_main()
{
   ESP_LOGI(TAG, "Init Arduino");
   initArduino();
   ESP_LOGI(TAG, "Init SSD");
   setupSSD1306();

   xTaskCreate(&blinkTask, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

   initSNTP();

   while (1) {
        if ( ((millis()/1000)>10) && (ui.getUiState()->currentFrame>0) ) { //wakeup for 10 seconds 
            dimSSD1306();
            const int deep_sleep_sec = 50;
            ESP_LOGW(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
            esp_deep_sleep(1000000LL * deep_sleep_sec); //Go sleep 50 seconds
            }
        ui.update();
        }
}


