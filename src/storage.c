#include <stm32f10x_flash.h>
#include <storage.h>
#include <eeprom.h>

typedef enum {
    STORAGE_ADDRESS_DEVICE_TYPE,
    STORAGE_ADDRESS_PAIRED_STATE,
    STORAGE_ADDRESS_ROUTER_BT_ADDRESS,
} storage_address_t;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[] = {
        0x0000,              /* 2 bytes, device type */
        0x0001,              /* 2 bytes, paired state */
        0x0002,              /* 6 bytes, 1 bt_address */
        0x0003,              /*          2 bt_address */
        0x0004,              /*          3 bt_address */
        0x0005,              /*          4 bt_address */
        0x0006,              /*          5 bt_address */
        0x0007,              /*          6 bt_address */
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
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_DEVICE_TYPE], type);
}

void storage_get_device_type(device_type_t *type)
{
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_DEVICE_TYPE],(uint16_t*)type);
}

bool storage_is_paired(void)
{
    uint16_t tmp;
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_PAIRED_STATE],&tmp);
    return tmp;
}

void storage_set_paired_state(bool paired)
{
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_PAIRED_STATE], paired);
}

void storage_set_router_bt_address(uint8_t *bt_address)
{
    uint16_t *address = (uint16_t*)bt_address;

    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS],address[0]);
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+1],address[1]);
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+2],address[2]);
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+3],address[3]);
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+4],address[4]);
    EE_WriteVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+5],address[5]);
}

void storage_get_router_bt_address(uint8_t *bt_address)
{
    uint16_t *address = (uint16_t*)bt_address;

    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS],&address[0]);
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+1],&address[1]);
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+2],&address[2]);
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+3],&address[3]);
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+4],&address[4]);
    EE_ReadVariable(VirtAddVarTab[STORAGE_ADDRESS_ROUTER_BT_ADDRESS+5],&address[5]);
}

