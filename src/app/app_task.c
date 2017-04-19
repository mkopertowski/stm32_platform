#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>
#include <projdefs.h>
#include <os.h>
#include <storage.h>
#include <io.h>
#include <disp.h>
#include <ADS1115.h>
#include <measurements.h>

#define DEBUG_ON
#include <debug.h>

struct context {
    TaskHandle_t app_task;
    TimerHandle_t app_timer;
};

static struct context ctx = { 0 };

static void button_state_listener(io_button_state_t state)
{
    switch(state) {
        case IO_BUTTON_SHORT_PRESS:
            OS_TASK_NOTIFY(ctx.app_task, APP_SHORT_PRESS_NOTIF);
            break;
        case IO_BUTTON_LONG_PRESS:
            OS_TASK_NOTIFY(ctx.app_task, APP_LONG_PRESS_NOTIF);
            break;
        default:
            DEBUG_PRINTF("APP: Unknown button state\r\n");
            break;
    }
}

static void module_hitted(void)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_MODULE_HITTED_NOTIF);
}

static void handle_module_hitted(void)
{
    DEBUG_PRINTF("APP: Module hitted\r\n");
}

static void handle_app_timer(TimerHandle_t xTimer)
{
    OS_TASK_NOTIFY(ctx.app_task, APP_ADS1115_READY_NOTIF);
}

void app_task(void *params)
{
    DEBUG_PRINTF("Application task started!\r\n");

    ctx.app_task = xTaskGetCurrentTaskHandle();

    /* register button state listener */
    io_button_register_listener(button_state_listener);

    DSP_vInit();
    DSP_vShowDisplay(DSP_ID_STARTUP_QUESTION);

    ctx.app_timer = xTimerCreate("app_tim",pdMS_TO_TICKS(1000),false,(void *)&ctx ,handle_app_timer);
    xTimerStart(ctx.app_timer, 0 );

    MES_vInit();

    for (;;) {
        BaseType_t ret;
        uint32_t notification;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret != pdPASS) {
            continue;
        }

        if(notification & APP_SHORT_PRESS_NOTIF) {
            DEBUG_PRINTF("APP: Short button press\r\n");
        }

        if(notification & APP_LONG_PRESS_NOTIF) {
            DEBUG_PRINTF("APP: Long button press\r\n");
            /* ToDo: reset device (enable disoverable mode, unpair, reboot */
        }

        if(notification & APP_MODULE_HITTED_NOTIF) {
            handle_module_hitted();
        }

        if(notification & APP_ADS1115_READY_NOTIF) {
            unsigned int channel = AINP_AIN0__AINN_GND;
            ADS1115_configure(start_one_conversion | channel | FS_6144mV | power_down_single_shot_mode | data_rate_860SPS | disable_comparator);
            DEBUG_PRINTF("ADS1115: value=%d\r\n", ADS1115_read(ADS1115_conversion_reg_pointer));
            xTimerStart(ctx.app_timer, 0 );
        }
    }
}
