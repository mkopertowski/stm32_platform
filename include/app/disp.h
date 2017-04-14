#ifndef DISP_H
#define DISP_H

typedef enum {

#define CREATE_DSP(A,B,C,D,E,F) A,
#include <app/disp_def.h>
#undef CREATE_DSP

  DSP_ID_LAST
} E_DSP_ID;

void DSP_vInit(void);
void DSP_vShowDisplay(E_DSP_ID);
void DSP_vShowTimerDisplay(E_DSP_ID, uint8_t);
void DSP_vUpdateDisplay(E_DSP_ID);

#endif /* DISP_H */
