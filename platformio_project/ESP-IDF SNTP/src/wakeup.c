/*
 * This sample illustrates how to go back to deep sleep from the
 * deep sleep wake stub.
 *
 * Consider the use case of counting pulses from an external sensor,
 * where the pulses arrive at a relatively slow rate.
 *
 * ESP32 is configured to go into deep sleep mode, and wake up from
 * a GPIO pin connected to the external pulse source.
 * Once the pulse arrives, ESP32 wakes up from deep sleep and runs
 * deep sleep wake stub. This stub function is stored in RTC fast
 * memory, so it can run without waiting for the whole firmware
 * to be loaded from flash.
 *
 * This function (called wake_stub below) increments the pulse counter,
 * stored in RTC_SLOW_MEM. This memory is also preserved when going
 * into deep sleep. Then the wake stub decides whether to continue
 * booting the firmware, or to go back to sleep. In this simple example,
 * the stub starts firmware when the pulse counter reaches 100.
 * Note: in real application, counting needs to be continued when the
 * application has started, for example using the PCNT peripheral.
 *
 */

#include <stdio.h>
#include <string.h>
#include "esp_sleep.h"
#include "esp_attr.h"
#include "rom/rtc.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/uart_reg.h"
#include "soc/timer_group_reg.h"

// Pin used for pulse counting
// GPIO0 is RTC_GPIO11 (see esp32_chip_pin_list_en.pdf)
#define PULSE_CNT_GPIO_NUM 0
#define PULSE_CNT_RTC_GPIO_NUM 11

#define PULSE_CNT_IS_LOW() \
    ((REG_GET_FIELD(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT) \
            & BIT(PULSE_CNT_RTC_GPIO_NUM)) == 0)

// Pulse counter value, stored in RTC_SLOW_MEM
static size_t RTC_DATA_ATTR s_pulse_count;
static size_t RTC_DATA_ATTR s_max_pulse_count;

// Function which runs after exit from deep sleep
static void RTC_IRAM_ATTR wake_stub();

void deepsleepWakeUpGPIO0(void)
{   /*
    if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) {
        printf("Wake up from deep sleep\n");
        printf("Pulse count=%d\n", s_pulse_count);
    } else {
        printf("Not a deep sleep wake up\n");

    }*/
    s_pulse_count = 0;
    s_max_pulse_count = 20;
    printf("Going to deep sleep in 1 second\n");
    printf("Will wake up after %d pulses\n", s_max_pulse_count);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    
    // Set the wake stub function
    esp_set_deep_sleep_wake_stub(&wake_stub);

    // Wake up on low logic level
    ESP_ERROR_CHECK( esp_sleep_enable_ext1_wakeup(
            1LL << PULSE_CNT_GPIO_NUM, ESP_EXT1_WAKEUP_ALL_LOW) );

    // Enter deep sleep
    esp_deep_sleep_start();
}

static const char RTC_RODATA_ATTR wake_fmt_str[] = "count=%d\n";
static const char RTC_RODATA_ATTR sleep_fmt_str[] = "sleeping\n";
static void RTC_IRAM_ATTR wake_stub()
{
    // Increment the pulse counter
    s_pulse_count++;
    // and print the pulse counter value:
    ets_printf(wake_fmt_str, s_pulse_count);

    if (s_pulse_count >= s_max_pulse_count) {
        // On revision 0 of ESP32, this function must be called:
        esp_default_wake_deep_sleep();

        // Return from the wake stub function to continue
        // booting the firmware.
        return;
    }
    // Pulse count is <s_max_pulse_count, go back to sleep
    // and wait for more pulses.

    // Wait for pin level to be high.
    // If we go to sleep when the pin is still low, the chip
    // will wake up again immediately. Hardware doesn't have
    // edge trigger support for deep sleep wakeup.
    do {
        while (PULSE_CNT_IS_LOW()) {
            // feed the watchdog
            REG_WRITE(TIMG_WDTFEED_REG(0), 1);
        }
        // debounce, 20ms
        ets_delay_us(20000);
    } while (PULSE_CNT_IS_LOW());

    // Print status
    ets_printf(sleep_fmt_str);
    // Wait for UART to end transmitting.
    while (REG_GET_FIELD(UART_STATUS_REG(0), UART_ST_UTX_OUT)) {
        ;
    }
    // Set the pointer of the wake stub function.
    REG_WRITE(RTC_ENTRY_ADDR_REG, (uint32_t)&wake_stub);
    // Go to sleep.
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
    // A few CPU cycles may be necessary for the sleep to start...
    while (true) {
        ;
    }
    // never reaches here.
}