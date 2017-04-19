//*****************************************************************************
/** \file    measurements.h
 *  \author  Mirek Kopertowski m.kopertowski/at/post.pl
 *  \brief
 */
//*****************************************************************************
#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <global.h>

void    MES_vInit(void);
void    MES_vHandler(void);
void    MES_vCalibrateSensorsInOxygen(void);
void    MES_vCalculateReadings(void);
uint8_t MES_ui8GetOxygenReading(uint8_t ui8SensorNr);
bool    MES_bCheckOxygenCell(uint8_t ui8SensorNr);
bool    MES_bCellWarning(void);

bool    MES_bIsCellEnabled(uint8_t ui8SensorNr);

#endif /* MEASUREMENTS_H */
