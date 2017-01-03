#include <stm32f10x_flash.h>
#include <storage.h>
#include <eeprom.h>

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[] = {
        0x0000,              /* device type */
        0x0001,              /* paired state */
};

void storage_init(void)
{
    /* Unlock the Flash Program Erase controller */
    FLASH_Unlock();

    /* EEPROM Init */
    EE_Init();
}

void storage_set_device_type(device_type_t type)
{
    EE_WriteVariable(VirtAddVarTab[0], type);
}

void storage_get_device_type(device_type_t *type)
{
    EE_ReadVariable(VirtAddVarTab[0],(uint16_t*)type);
}

bool storage_is_paired(void)
{
    uint16_t tmp;
    EE_ReadVariable(VirtAddVarTab[1],&tmp);
    return tmp;
}

void storage_set_paired_state(bool paired)
{
    EE_WriteVariable(VirtAddVarTab[0], paired);
}


