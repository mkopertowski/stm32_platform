/*
CREATE_DSP(display id
text1,
text2,
text3 <===> softkeytext1, softkeytext2)
*/

CREATE_DSP(DSP_ID_STARTUP, \
TXT_ID_O2CONSOLE, \
TXT_ID_FOR_MCCR, \
TXT_ID_VERSION, \
0,0 \
)

CREATE_DSP(DSP_ID_STARTUP_QUESTION, \
TXT_ID_SWITCH_ON, \
0, \
0, \
TXT_ID_YES, TXT_ID_NO \
)

CREATE_DSP(DSP_ID_CALIBRATION_QUESTION, \
TXT_ID_CALIBARTION_QUESTION, \
TXT_ID_CELLS3, \
0, \
TXT_ID_NO, TXT_ID_YES  \
)

CREATE_DSP(DSP_ID_CALIBRATION, \
0, \
TXT_ID_CALIBARTION_1, \
0, \
TXT_ID_CALIBRATE, TXT_ID_SKIP \
)

CREATE_DSP(DSP_ID_ENTER_DIVEMODE, \
0, \
TXT_ID_ENTERING_DIVEMODE, \
0, \
0,0 \
)

CREATE_DSP(DSP_ID_OXYGEN, \
TXT_ID_OXYGEN, \
0, \
0, \
TXT_ID_SET, TXT_ID_PWR \
)

CREATE_DSP(DSP_ID_POWER_DOWN, \
0, \
TXT_ID_POWER_DOWN, \
0, \
0, 0 \
)

CREATE_DSP(DSP_ID_AMBIENT_PRESSURE, \
TXT_ID_ATMPRESSURE, \
TXT_ID_CHANGE, \
0, \
TXT_ID_YES, TXT_ID_NO \
)

CREATE_DSP(DSP_ID_AMBIENT_PRESSURE_SET, \
TXT_ID_ATMPRESSURE, \
0, \
0, \
TXT_ID_DOWN, TXT_ID_UP \
)

CREATE_DSP(DSP_ID_AMBIENT_PRESSURE_SAVED, \
TXT_ID_ATMPRESSURE, \
TXT_ID_SAVED, \
0, \
0, 0 \
)

CREATE_DSP(DSP_ID_SETTINGS_02CELLS, \
TXT_ID_SETTINGS, \
0, \
0, \
TXT_ID_TOGGLE, 0 \
)
