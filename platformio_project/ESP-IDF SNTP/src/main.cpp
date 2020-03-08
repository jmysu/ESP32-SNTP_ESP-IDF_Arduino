
#include "Arduino.h"
#include <OLEDDisplayUi.h>

static const char *TAG = "main";

extern OLEDDisplayUi ui;
extern "C" void initSNTP();
extern void setupSSD();

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
   setupSSD();

   xTaskCreate(&blinkTask, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

   initSNTP();

   while (1) {
        ui.update();
        }
}


