#ifndef BT740_H_
#define BT740_H_

#include <global.h>

#define RESPONSE_DATA_LENGTH (30)
#define BT_ADDRESS_LENGTH (12)

typedef enum {
    BT_MODULE_OFFLINE,
    BT_MODULE_READY,
} bt_state_t;

typedef enum {
    BT_CMD_UNKNOWN,
    BT_CMD_STATUS_CHECK,
    BT_CMD_ECHO_OFF,
    BT_CMD_GET_DEVICES,
    BT_CMD_WIRTE_S_REGISTER,
    BT_CMD_GET_FRIENDLY_NAME,
    BT_CMD_WRITE_SREG_DISCOVERABLE,
    BT_CMD_WRITE_SREG_CONNECTABLE,
    BT_CMD_LAST,
} bt_cmd_type_t;

typedef union {
    uint8_t param;
    uint8_t bt_address[BT_ADDRESS_LENGTH];
} bt_cmd_params_t;

typedef struct {
    bt_cmd_type_t type;
    bt_cmd_params_t params;
} bt_cmd_t;

typedef struct response_queue {
    uint8_t data[RESPONSE_DATA_LENGTH];
    struct response_queue *next;
} response_queue_t;

typedef void (*message_cb)(uint8_t *data, uint8_t data_len);
typedef void (*response_cb)(bool status, response_queue_t *resp);
typedef void (*state_cb)(bt_state_t state);

void BT740_init(void);
void BT740_sendCmd(bt_cmd_t *cmd, response_cb cb);
void BT740_register_for_packets(message_cb cb);
void BT740_send_packet(uint8_t *data, uint8_t data_len);

void BT740_register_for_state(state_cb cb);

#endif // BT740_H
