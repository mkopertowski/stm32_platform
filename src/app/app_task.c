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
#include <config.h>
#include <storage.h>
#include <disp.h>
#include <measurements.h>
#include <platform/io.h>
#include <platform/sys.h>

#define DEBUG_ON
#include <debug.h>

typedef enum {
    APP_ST_INIT,
    APP_ST_STARTUP_QUESTION,
    APP_ST_STARTUP,
    APP_ST_CALIBRATION_QUESTION,
    APP_ST_AMBIENT_PRESSURE_QUESTION,
    APP_ST_AMBIENT_PRESSURE_SET,
    APP_ST_AMBIENT_PRESSURE_SET_TIMEOUT,
    APP_ST_CALIBRATION,
    APP_ST_CALIBRATED,
    APP_ST_DIVEMODE,
    APP_ST_POWERDOWN,
    APP_ST_SETTINGS_O2CELLS
} APP_E_STATES;

typedef enum {
    APP_EV_FOCUS_ON,
    APP_EV_TIMEOUT,
    APP_EV_LEFT_KEY,
    APP_EV_RIGHT_KEY,
    APP_EV_BATMON      // battery monitor
} APP_E_EVENTS;

struct context {
    TaskHandle_t task_handle;
    TimerHandle_t disp_update_timer;

    APP_E_STATES  eAppState;
    uint8_t ui8SensorNr;
};

static struct context ctx = { 0 };

static void vApplication(uint32_t notif);
static APP_E_EVENTS eReadEvent(uint32_t notif);
static void vHandleStartup(APP_E_EVENTS);
static void vHandleCalibration(APP_E_EVENTS);
static void vHandleDiveMode(APP_E_EVENTS);
static void vHandleLEDs(void);
static void vHandleSettings(APP_E_EVENTS);

static void vResetAppTimer(void)
{

}

static void vSetAppTimer(uint8_t seconds)
{

}

static bool bPeriodicTasks(uint16_t period)
{
    return false;
}

static void button_state_listener(io_button_state_t state)
{
    switch(state) {
        case IO_BUTTON_SHORT_PRESS:
            OS_TASK_NOTIFY(ctx.task_handle, APP_SHORT_PRESS_LEFT_KEY_NOTIF);
            break;
        case IO_BUTTON_LONG_PRESS:
            OS_TASK_NOTIFY(ctx.task_handle, APP_LONG_PRESS_LEFT_KEY_NOTIF);
            break;
        default:
            DEBUG_PRINTF("APP: Unknown button state\r\n");
            break;
    }
}

static void handle_display_update_timer(TimerHandle_t xTimer)
{
    OS_TASK_NOTIFY(ctx.task_handle, MES_READ_O2CELLS_NOTIF);
}

void app_task(void *params)
{
    DEBUG_PRINTF("Application task started!\r\n");

    ctx.task_handle = xTaskGetCurrentTaskHandle();

    ctx.eAppState = APP_ST_INIT;

    /* register button state listener */
    io_button_register_listener(button_state_listener);

    DSP_vInit();
    DSP_vShowDisplay(DSP_ID_STARTUP_QUESTION);

    ctx.disp_update_timer = xTimerCreate("app_tim",pdMS_TO_TICKS(1000),false,(void *)&ctx ,handle_display_update_timer);

    MES_vInit();

    for (;;) {
        BaseType_t ret;
        uint32_t notification;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret != pdPASS) {
            continue;
        }

        if((notification & APP_SHORT_PRESS_LEFT_KEY_NOTIF) ||
           (notification & APP_LONG_PRESS_LEFT_KEY_NOTIF) ||
           (notification & APP_SHORT_PRESS_RIGHT_KEY_NOTIF) ||
           (notification & APP_LONG_PRESS_RIGHT_KEY_NOTIF) ||
           (notification & APP_TIMEOUT_NOTIF)) {
            vApplication(notification);
        }
    }
}

static void vApplication(uint32_t notif)
{
    APP_E_EVENTS eAppEvent = eReadEvent(notif);

    switch(ctx.eAppState)
    {
        case APP_ST_INIT:
        case APP_ST_STARTUP_QUESTION:
        case APP_ST_STARTUP:
            vHandleStartup(eAppEvent);
            break;
        case APP_ST_CALIBRATION_QUESTION:
        case APP_ST_CALIBRATION:
        case APP_ST_CALIBRATED:
        case APP_ST_AMBIENT_PRESSURE_QUESTION:
        case APP_ST_AMBIENT_PRESSURE_SET:
        case APP_ST_AMBIENT_PRESSURE_SET_TIMEOUT:
            // oxygen measurements handler
            vHandleCalibration(eAppEvent);
            break;
        case APP_ST_DIVEMODE:
        case APP_ST_POWERDOWN:
            vHandleDiveMode(eAppEvent);
            break;
        case APP_ST_SETTINGS_O2CELLS:
            vHandleSettings(eAppEvent);
        default:
            break;
    }
}

static APP_E_EVENTS eReadEvent(uint32_t notif)
{
    APP_E_EVENTS eEvent = APP_EV_FOCUS_ON;

    if(notif & APP_SHORT_PRESS_LEFT_KEY_NOTIF)
    {
        eEvent = APP_EV_LEFT_KEY;
    }
    else if(notif & APP_SHORT_PRESS_RIGHT_KEY_NOTIF)
    {
        eEvent = APP_EV_RIGHT_KEY;
    }
    else if(notif & APP_TIMEOUT_NOTIF)
    {
        eEvent = APP_EV_TIMEOUT;
    }

    return eEvent;
}

static void vHandleStartup(APP_E_EVENTS eEvent)
{
    if(eEvent != APP_EV_FOCUS_ON)
    {
        DEBUG_PRINTF("STATE=%d, EVENT=%d\n",ctx.eAppState,eEvent);
    }
    switch(ctx.eAppState)
    {
        case APP_ST_INIT:
            // blink all leds
            //IO_vSetPortState(LED_RED_UP,IO_STATE_ON);
            //IO_vSetPortState(LED_RED_DOWN,IO_STATE_ON);
            //IO_vSetPortState(LED_GREEN,IO_STATE_ON);
            if(storage_is_diveMode())
            {
                ctx.eAppState = APP_ST_DIVEMODE;
                DSP_vShowDisplay(DSP_ID_OXYGEN);
            }
            else
            {
                DSP_vShowTimerDisplay(DSP_ID_STARTUP,3);
                ctx.eAppState = APP_ST_STARTUP_QUESTION;
            }
            break;

        case APP_ST_STARTUP_QUESTION:
            if(eEvent==APP_EV_TIMEOUT)
            {
                DSP_vShowTimerDisplay(DSP_ID_STARTUP_QUESTION,5);
                ctx.eAppState = APP_ST_STARTUP;
            }
            break;

        case APP_ST_STARTUP:
            switch(eEvent)
            {
                case APP_EV_TIMEOUT:
                case APP_EV_RIGHT_KEY:      // NO
                    ctx.eAppState = APP_ST_INIT;
                    SYS_vPowerDown();
                    break;

                case APP_EV_LEFT_KEY:       // YES
                    // init measurements
                    MES_vInit();
                    ctx.eAppState = APP_ST_CALIBRATION_QUESTION;
                    DSP_vShowTimerDisplay(DSP_ID_CALIBRATION_QUESTION,4);
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

static void vHandleCalibration(APP_E_EVENTS eEvent)
{
    static uint16_t ui16AtmPressure;

    switch(ctx.eAppState)
    {
        case APP_ST_CALIBRATION_QUESTION:
            switch(eEvent)
            {
                case APP_EV_TIMEOUT:
                case APP_EV_LEFT_KEY:
                    ctx.eAppState = APP_ST_CALIBRATED;
                    DSP_vShowTimerDisplay(DSP_ID_ENTER_DIVEMODE,3);
                    break;

                case APP_EV_RIGHT_KEY:
                    vResetAppTimer();
                    ui16AtmPressure = storage_get_atmospherePressure();
                    ctx.eAppState = APP_ST_AMBIENT_PRESSURE_QUESTION;
                    DSP_vShowDisplay(DSP_ID_AMBIENT_PRESSURE);
                    break;

                default:
                    break;
            }
            break;

        case APP_ST_AMBIENT_PRESSURE_QUESTION:
            switch(eEvent)
            {
                case APP_EV_LEFT_KEY:
                    ctx.eAppState = APP_ST_AMBIENT_PRESSURE_SET;
                    DSP_vShowDisplay(DSP_ID_AMBIENT_PRESSURE_SET);
                    break;

                case APP_EV_RIGHT_KEY:
                    ctx.eAppState = APP_ST_CALIBRATION;
                    DSP_vShowDisplay(DSP_ID_CALIBRATION);
                    break;

                default:
                    break;
            }
            break;

        case APP_ST_AMBIENT_PRESSURE_SET:
            switch(eEvent)
            {
                case APP_EV_RIGHT_KEY:
                    ui16AtmPressure+=1;
                    vSetAppTimer(5);
                    DSP_vUpdateDisplay(DSP_ID_AMBIENT_PRESSURE_SET);
                    break;

                case APP_EV_LEFT_KEY:
                    ui16AtmPressure-=1;
                    vSetAppTimer(5);
                    DSP_vUpdateDisplay(DSP_ID_AMBIENT_PRESSURE_SET);
                    break;

                case APP_EV_TIMEOUT:
                    ctx.eAppState = APP_ST_AMBIENT_PRESSURE_SET_TIMEOUT;
                    storage_set_atmospherePressure(ui16AtmPressure);
                    DSP_vShowTimerDisplay(DSP_ID_AMBIENT_PRESSURE_SAVED,3);
                    break;

                default:
                    break;
            }
            break;

        case APP_ST_AMBIENT_PRESSURE_SET_TIMEOUT:
            if(eEvent == APP_EV_TIMEOUT)
            {
                ctx.eAppState = APP_ST_CALIBRATION;
                DSP_vShowDisplay(DSP_ID_CALIBRATION);
            }
            break;

        case APP_ST_CALIBRATION:
            switch(eEvent)
            {
                case APP_EV_FOCUS_ON:
                    // every 1000ms = 1s
                    if(bPeriodicTasks(100))
                    {
                        DSP_vUpdateDisplay(DSP_ID_CALIBRATION);
                    }
                    break;
                case APP_EV_LEFT_KEY:  // calibrate
                    MES_vCalibrateSensorsInOxygen();
                case APP_EV_RIGHT_KEY: // skip
                    ctx.eAppState = APP_ST_CALIBRATED;
                    DSP_vShowTimerDisplay(DSP_ID_ENTER_DIVEMODE,3);
                    break;

                default:
                    break;
            }
            break;

        case APP_ST_CALIBRATED:
            if(eEvent == APP_EV_TIMEOUT)
            {
                ctx.eAppState = APP_ST_DIVEMODE;
                storage_set_diveMode(true);
                DSP_vShowDisplay(DSP_ID_OXYGEN);
            }
            break;

        default:
            break;
    }
}

static void vHandleDiveMode(APP_E_EVENTS eEvent)
{
    uint32_t ui32BaterryLevel;
    uint16_t ui16BatteryRead;
    uint8_t ui8BatteryLevel;
    uint16_t ui16BatteryFactor = storage_get_batteryMonitorFactor();

    switch(ctx.eAppState)
    {
        case APP_ST_DIVEMODE:
            switch(eEvent)
            {
                case APP_EV_FOCUS_ON:

                    // every 100ms -> get cell reading
                    if(bPeriodicTasks(100))
                    {
                        // oxygen measurements handler
                        //ToDo MES_vHandler();
                    }
                    // every 500ms -> handle oxygen level LEDs
                    if(bPeriodicTasks(1000))
                    {
                        //ToDo  vHandleLEDs();
                    }
                    // every 0.5s -> update display
                    if(bPeriodicTasks(500))
                    {
                        DSP_vUpdateDisplay(DSP_ID_OXYGEN);
                    }
                    // every 60s -> monitor battery level
                    if(bPeriodicTasks(60000))
                    {
                        //ToDo ADC_vStartConversion();
                    }
                    break;

                case APP_EV_RIGHT_KEY: // "off"
                    storage_set_diveMode(false);
                    ctx.eAppState = APP_ST_POWERDOWN;
                    DSP_vShowTimerDisplay(DSP_ID_POWER_DOWN,3);
                    break;

                case APP_EV_LEFT_KEY: // "off"
                    storage_set_diveMode(false);
                    ctx.ui8SensorNr = 0;
                    vSetAppTimer(4);
                    ctx.eAppState = APP_ST_SETTINGS_O2CELLS;
                    DSP_vShowDisplay(DSP_ID_SETTINGS_02CELLS);
                    break;

                case APP_EV_BATMON:
                    //ToDo ui16BatteryRead = ADC_ui16Read();
                    ui32BaterryLevel = (uint32_t)ui16BatteryRead * GL_BATTERY_FACTOR_MAX;
                    ui32BaterryLevel = ui32BaterryLevel / (uint32_t)ui16BatteryFactor;
                    if(ui32BaterryLevel>GL_BATTERY_100)
                    {
                        ui8BatteryLevel = 100;
                    }
                    else if(ui32BaterryLevel>GL_BATTERY_75)
                    {
                        ui8BatteryLevel = 75;
                    }
                    else if(ui32BaterryLevel>GL_BATTERY_50)
                    {
                        ui8BatteryLevel = 50;
                    }
                    else if(ui32BaterryLevel>GL_BATTERY_25)
                    {
                        ui8BatteryLevel = 25;
                    }
                    else
                    {
                        ui8BatteryLevel = 0;
                    }
                    DEBUG_PRINTF("Battery Level=%d, read ADC=%d %ld\n",ui8BatteryLevel,ui16BatteryRead,ui32BaterryLevel);
                    break;

                default:
                    break;
            }
            break;

        case APP_ST_POWERDOWN:
            if(eEvent == APP_EV_TIMEOUT)
            {
                ctx.eAppState = APP_ST_INIT;
                SYS_vPowerDown();
            }
            break;

        default:
            break;
    }
}

static void HeadUPLedForSensor(uint8_t nr)
{
    if(MES_ui8GetOxygenReading(nr) > GL_PPO2_TOOHIGH_WARNING)
    {
        io_set_headup_led(LED_RED_UP,IO_STATE_ON);
    }
    else if(MES_ui8GetOxygenReading(nr) < GL_PPO2_TOOLOW_WARNING)
    {
        io_set_headup_led(LED_GREEN,IO_STATE_ON_SHORT);
        io_set_headup_led(LED_RED_UP,IO_STATE_ON_LONG);
    }
    else if(MES_ui8GetOxygenReading(nr) < GL_PPO2_LOW_WARNING)
    {
        io_set_headup_led(LED_GREEN,IO_STATE_ON_SHORT);
    }
    else if(MES_ui8GetOxygenReading(nr) > GL_PPO2_HIGH_WARNING)
    {
        io_set_headup_led(LED_RED_UP,IO_STATE_ON);
        io_set_headup_led(LED_GREEN,IO_STATE_ON);
    }
    else // OK
    {
        io_set_headup_led(LED_GREEN,IO_STATE_ON);
    }
}

static void vHandleLEDs(void)
{
    if(MES_bIsCellEnabled(0) && MES_bIsCellEnabled(1))
    {
        if((MES_ui8GetOxygenReading(0) > GL_PPO2_TOOHIGH_WARNING) ||
           (MES_ui8GetOxygenReading(1) > GL_PPO2_TOOHIGH_WARNING))
        {
            io_set_headup_led(LED_RED_UP,IO_STATE_ON);
        }
        else if((MES_ui8GetOxygenReading(0) < GL_PPO2_TOOLOW_WARNING) ||
                (MES_ui8GetOxygenReading(1) < GL_PPO2_TOOLOW_WARNING))
        {
            io_set_headup_led(LED_GREEN,IO_STATE_ON_SHORT);
            io_set_headup_led(LED_RED_UP,IO_STATE_ON_LONG);
        }
        else if((MES_ui8GetOxygenReading(0) < GL_PPO2_LOW_WARNING) ||
                (MES_ui8GetOxygenReading(1) < GL_PPO2_LOW_WARNING))
        {
            io_set_headup_led(LED_GREEN,IO_STATE_ON_SHORT);
        }
        else if((MES_ui8GetOxygenReading(0) > GL_PPO2_HIGH_WARNING) ||
                (MES_ui8GetOxygenReading(1) > GL_PPO2_HIGH_WARNING))
        {
            io_set_headup_led(LED_RED_UP,IO_STATE_ON);
            io_set_headup_led(LED_GREEN,IO_STATE_ON);
        }
        else // OK
        {
            io_set_headup_led(LED_GREEN,IO_STATE_ON);
        }
    }
    else if(MES_bIsCellEnabled(0))
    {
        HeadUPLedForSensor(0);
    }
    else if(MES_bIsCellEnabled(1))
    {
        HeadUPLedForSensor(1);
    }
    else
    {

    }
}

static void vHandleSettings(APP_E_EVENTS eEvent)
{
    bool bIsCellEnabled;
    static uint8_t ui8SensorNr = 0;

    switch(ctx.eAppState)
    {
        case APP_ST_SETTINGS_O2CELLS:
            switch(eEvent)
            {
                case APP_EV_LEFT_KEY:
                    bIsCellEnabled = storage_isSensorEnabled(ui8SensorNr);
                    bIsCellEnabled = !bIsCellEnabled;
                    storge_set_sensorEnable(ui8SensorNr,bIsCellEnabled);
                    vSetAppTimer(4);
                    DSP_vUpdateDisplay(DSP_ID_SETTINGS_02CELLS);
                    break;

                case APP_EV_TIMEOUT:
                    ui8SensorNr++;
                    if(ui8SensorNr<GL_NR_OF_O2CELLS)
                    {
                        vSetAppTimer(4);
                        DSP_vUpdateDisplay(DSP_ID_SETTINGS_02CELLS);
                    }
                    else
                    {
                        // last cell set -> power down
                        ui8SensorNr = 0;
                        ctx.eAppState = APP_ST_POWERDOWN;
                        DSP_vShowTimerDisplay(DSP_ID_POWER_DOWN,3);
                    }
                    break;

                default:
                    break;
            }
            break;
        default:
            break;
    }
}
