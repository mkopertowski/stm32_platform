#ifndef BT740_H_
#define BT740_H_

#include "stm32f10x.h"
#include "global.h"

typedef enum {
    BT_CMD_UNKNOWN,
    BT_CMD_STATUS_CHECK,
    BT_CMD_WRITE_SREG_DISCOVERABLE,
    BT_CMD_WRITE_SREG_CONNECTABLE,
    BT_CMD_LAST,
} bt_cmd_type_t;

typedef union {
    uint8_t param;
} bt_cmd_params_t;

typedef struct {
    bt_cmd_type_t type;
    bt_cmd_params_t params;
} bt_cmd_t;

typedef union {
    uint8_t error_code;
} bt_cmd_response_t;

void BT740_init(void);
bool BT740_sendCmd(bt_cmd_t *cmd, bt_cmd_response_t *response);

#endif // BT740
