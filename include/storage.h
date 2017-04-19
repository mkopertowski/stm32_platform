#ifndef STORAGE_H_
#define STORAGE_H_

#include <global.h>

typedef enum {
    DEVICE_TYPE_ROUTER,
    DEVICE_TYPE_AVALANCHE_BEACON,
} device_type_t;

void storage_init(void);

uint16_t storage_get_sensorCalibrationFactor(uint8_t cellNr);
void storage_set_sensorCalibrationFactor(uint8_t cellNr, uint16_t val);

bool storage_isSensorEnabled(uint8_t cellNr);

#endif
