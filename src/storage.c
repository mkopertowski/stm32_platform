#include <stm32f10x_flash.h>
#include <storage.h>

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[] = {
        0x0000,              /* device type */
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
    EE_ReadVariable(VirtAddVarTab[0],type);
}
