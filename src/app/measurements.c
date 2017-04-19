//*****************************************************************************
/** \file    meassure.c
 *  \author  Mirek Kopertowski m.kopertowski/at/post.pl
 *  \brief   Oxygen partial pressure measurement
 */
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>

#include <global.h>
#include <config.h>
#include <os.h>

#include <storage.h>

#include <drivers/ADS1115.h>

#define DEBUG_ON
#include <debug.h>

/** CALCULATIONS
 *
 *  przy zakresie 256mV = 2^8mV mam 15 bitow
 *  potrzebuje mierzyc do 100mV
 *  dokldanosc mam 2^8 / 2^15 = 2^-7 = 0,0078125mV
 *
 *  1) Resolution is set to 16bits,
 *
 *  2) output code = 32768 * 8 * V[mV] / 2048mV = 2^15 * 2^3 * V / 2^11 =
 *                 = 2^7 * V[mV] = 128 * V[mV]
 *
 *  3) V [mv] = output code / 128
 *
 *  4) cell range:
 *
 *     MES_CELL_MIN = 128 * 5[mV] = 640
 */


//***** DEFINES ***************************************************************
#define MES_NR_OF_MEASUREMENTS    5   ///< configurations
#define MES_CELL_MIN              640 ///< oxygen cell minimum value (5mV)

typedef struct {
    uint16_t aui16CalibrationFactor;
    bool bCorrectRange;
    bool bIsEnabled;
    int16_t ai16Measurements[MES_NR_OF_MEASUREMENTS];
    uint8_t aui8OxygenReading;
} ST_O2SENSOR;

struct context {
    TaskHandle_t task_handle;
    TimerHandle_t timer;

    uint8_t ui8CurrentCellNr;
    uint8_t ui8MesurementNr;
    ST_O2SENSOR sensor[GL_NR_OF_O2CELLS];
};

static struct context ctx = { 0 };

//***** STATIC FUNCTIONS DECLARATIONS *****************************************
static uint16_t ui16CalculateMeanForSensor(uint8_t);
static void reado2Cell(uint8_t);

void MES_task(void *params);

void MES_vInit(void)
{
    uint8_t ui8CellNr;

    ctx.ui8CurrentCellNr= 0;
    ctx.ui8MesurementNr = 0;

    for(ui8CellNr=0;ui8CellNr<GL_NR_OF_O2CELLS;ui8CellNr++)
    {
        ctx.sensor[ui8CellNr].aui16CalibrationFactor = storage_get_sensorCalibrationFactor(ui8CellNr);

        DEBUG_PRINTF("cel=%d, F=%d\n",ui8CellNr,ctx.sensor[ui8CellNr].aui16CalibrationFactor);

        ctx.sensor[ui8CellNr].bCorrectRange = false;
        ctx.sensor[ui8CellNr].bIsEnabled = storage_isSensorEnabled(ui8CellNr);
    }

    ADS1115_init();

    xTaskCreate(MES_task, "mes_task", 512, NULL, OS_TASK_PRIORITY, NULL);
}

static void handle_app_timer(TimerHandle_t xTimer)
{
    OS_TASK_NOTIFY(ctx.task_handle, MES_READ_O2CELLS_NOTIF);
}

static void handleReado2Cells(void)
{
    uint8_t i;

    /* read o2 cells */
    for(i=0;i<GL_NR_OF_O2CELLS;i++) {
        reado2Cell(i);
    }

    /* restart timer to read cells in a while */
    xTimerStart(ctx.timer,0);
}

void MES_task(void *params)
{
    uint16_t ui16val;

    ctx.task_handle = xTaskGetCurrentTaskHandle();

    ctx.timer = xTimerCreate("mes_tim",pdMS_TO_TICKS(GL_O2CELLS_MEASURE_INTERVAL_MS),false,(void *)&ctx ,handle_app_timer);
    xTimerStart(ctx.timer, 0 );

    for (;;) {
        BaseType_t ret;
        uint32_t notification;

        /* Wait on any of the event group bits, then clear them all */
        ret = xTaskNotifyWait(0, OS_TASK_NOTIFY_MASK, &notification, pdMS_TO_TICKS(50));

        if(ret != pdPASS) {
            continue;
        }

        if(notification & MES_READ_O2CELLS_NOTIF) {
            DEBUG_PRINTF("MES: Reading o2 cells\r\n");
            handleReado2Cells();
        }
    }
}

void MES_vCalibrateSensorsInOxygen(void)
{
    uint8_t ui8SensorNr;
    uint16_t ui16CalibrationFactor;
    uint32_t val;
    uint16_t ui16AtmPressure = 1013;

    ui16AtmPressure = storage_get_atmospherePressure();

    for(ui8SensorNr=0;ui8SensorNr<GL_NR_OF_O2CELLS;ui8SensorNr++)
    {
        // get the average value in mV
        val = ui16CalculateMeanForSensor(ui8SensorNr);

        // adjust calibaration factor to the atmospheric pressure:
        val = val * 1013;
        val = val / ui16AtmPressure;
        // assume calibaion is in 99% of oxygen
        val = val * 100;
        val = val / 99;

        ui16CalibrationFactor = (uint16_t) val;

        DEBUG_PRINTF("cel=%d, F=%d at atm. pressure %d\n",ui8SensorNr,ui16CalibrationFactor,ui16AtmPressure);
        // the factor is the output in mV for
        // 100% i.e 1ATA of oxygen
        // store in eeprom
        storage_set_sensorCalibrationFactor(ui8SensorNr,ui16CalibrationFactor);
        // set new calibration facotrs for application
        ctx.sensor[ui8SensorNr].aui16CalibrationFactor = ui16CalibrationFactor;
    }
}

uint8_t MES_ui8GetOxygenReading(uint8_t ui8SensorNr)
{
    return ctx.sensor[ui8SensorNr].aui8OxygenReading;
}

bool MES_bCheckOxygenCell(uint8_t ui8CellNr)
{
    return ctx.sensor[ui8CellNr].bCorrectRange;
}

//*****************************************************************************
/** \brief   Calculate oxygen readings
 *
 *           Function should be called before MES_ui8GetOxygenReading() when
 *           it is required to have new values
 *  \return  -
 */
//*****************************************************************************
void MES_vCalculateReadings(void)
{
    uint8_t ui8SensorNr;
    uint32_t val;

    for(ui8SensorNr=0;ui8SensorNr<GL_NR_OF_O2CELLS;ui8SensorNr++)
    {
        val = ui16CalculateMeanForSensor(ui8SensorNr);
        DEBUG_PRINTF("cel=%d, mean=%u\n",ui8SensorNr,val);
        val = val * 100;
        val = val / ctx.sensor[ui8SensorNr].aui16CalibrationFactor;

        ctx.sensor[ui8SensorNr].aui8OxygenReading = (uint8_t)val;
        DEBUG_PRINTF("reading=%u\n",val);
    }
}

bool MES_bCellWarning(void)
{
    uint8_t ui8SensorNr;
    bool bWarning = false;

    for(ui8SensorNr=0;ui8SensorNr<GL_NR_OF_O2CELLS;ui8SensorNr++)
    {
        if((ctx.sensor[ui8SensorNr].aui8OxygenReading<GL_LOW_PPO2_WARNING) ||
           (ctx.sensor[ui8SensorNr].aui8OxygenReading>GL_HIGH_PPO2_WARNING))
        {
            bWarning = true;
            break;
        }
    }
    return bWarning;
}

static uint16_t ui16CalculateMeanForSensor(uint8_t ui8SensorNr)
{
    uint8_t ui8MesurementNr;
    int16_t min,max;
    int32_t sum;
    uint16_t ui16output;

    min = max = sum = ctx.sensor[ui8SensorNr].ai16Measurements[0];
    for(ui8MesurementNr=1;ui8MesurementNr<MES_NR_OF_MEASUREMENTS;ui8MesurementNr++)
    {
        sum += ctx.sensor[ui8SensorNr].ai16Measurements[ui8MesurementNr];
        if(ctx.sensor[ui8SensorNr].ai16Measurements[ui8MesurementNr] < min)
        {
            // set min
            min = ctx.sensor[ui8SensorNr].ai16Measurements[ui8MesurementNr];
        }
        else if(ctx.sensor[ui8SensorNr].ai16Measurements[ui8MesurementNr] > max)
        {
            // set max
            max = ctx.sensor[ui8SensorNr].ai16Measurements[ui8MesurementNr];
        }
    }
    if(MES_NR_OF_MEASUREMENTS > 4)
    {
        // assuming that 5 is minimum number of samples having sens to
        // exclude min and max values
        if(min != max)
        {
            sum -= max + min;
            sum = sum / (MES_NR_OF_MEASUREMENTS - 2);
        }
        else
        {
            sum = sum / MES_NR_OF_MEASUREMENTS;
        }
    }
    else
    {
        sum = sum / MES_NR_OF_MEASUREMENTS;
    }

    // sum should fit well in 16 bits beacuse it is average value
    ui16output = (uint16_t)sum;

    return ui16output;
}

static void reado2Cell(uint8_t ui8CellNr)
{
    unsigned int channel = AINP_AIN0__AINN_GND;
    uint8_t i;

    switch(ui8CellNr) {
        case 0:
            channel = AINP_AIN0__AINN_GND;
            break;
        case 1:
            channel = AINP_AIN1__AINN_GND;
            break;
        case 2:
            channel = AINP_AIN2__AINN_GND;
            break;
    }

    for(i=0;i<MES_NR_OF_MEASUREMENTS;i++) {
        ADS1115_configure(start_one_conversion | channel | FS_256mV | power_down_single_shot_mode | data_rate_860SPS | disable_comparator);
        ctx.sensor[ui8CellNr].ai16Measurements[i] = ADS1115_read(ADS1115_conversion_reg_pointer);
    }
}

bool MES_bIsCellEnabled(uint8_t ui8SensorNr)
{
    return ctx.sensor[ui8SensorNr].bIsEnabled;
}

