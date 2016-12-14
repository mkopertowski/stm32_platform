#ifndef BT740_H_
#define BT740_H_

#include "stm32f10x.h"
#include "global.h"

typedef enum {
    BT_CMD_UNKNOWN,
    BT_CMD_STATUS_CHECK,
    BT_CMD_LAST,
} BT_CMD_TYPE;

typedef union {
    uint8_t param;
} BT_CMD_PARAMS;

typedef struct {
    BT_CMD_TYPE type;
    BT_CMD_PARAMS params;
} BT_CMD;

typedef union {
    uint8_t error_code;
} BT_RESPONSE;

void BT740_init(void);
bool BT740_sendCmd(BT_CMD *cmd, BT_RESPONSE *response);

#endif // BT740
