#include <stm32f10x_flash.h>
#include <storage.h>
#include <eeprom.h>

typedef enum {
    STORAGE_ADDRESS_CELL1_CALIBRATION_FACTOR,
    STORAGE_ADDRESS_CELL2_CALIBRATION_FACTOR,
    STORAGE_ADDRESS_CELL_ENABLED,
} storage_address_t;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[] = {
        0x0000,              /* 2 bytes, cell 1 calibration factor */
        0x0001,              /* 2 bytes, cell 2 calibration factor */
        0x0002,
        0x0003,
        0x0004,
        0x0005,
        0x0006,
        0x0007,
};

void storage_init(void)
{
    /* Unlock the Flash Program Erase controller */
    FLASH_Unlock();

    /* EEPROM Init */
    EE_Init();
}

uint16_t storage_get_sensorCalibrationFactor(uint8_t cellNr)
{
    uint16_t tmp_val;

    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_CELL1_CALIBRATION_FACTOR]+cellNr,(uint16_t*)&tmp_val);

    return tmp_val;
}

void storage_set_sensorCalibrationFactor(uint8_t cellNr, uint16_t val)
{
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_CELL1_CALIBRATION_FACTOR]+cellNr, val);
}

bool storage_isSensorEnabled(uint8_t cellNr)
{
    return true;
}
