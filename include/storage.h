#ifndef STORAGE_H_
#define STORAGE_H_

#include <global.h>

typedef enum {
    DEVICE_TYPE_ROUTER,
    DEVICE_TYPE_AVALANCHE_BEACON,
} device_type_t;

void storage_init(void);

void storage_set_device_type(device_type_t type);
void storage_get_device_type(device_type_t *type);

bool storage_is_paired(void);
void storage_set_paired_state(bool paired);

#endif
