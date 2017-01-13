#ifndef BT740_H_
#define BT740_H_

#include <global.h>

#define RESPONSE_DATA_LENGTH (30)

typedef enum {
    BT_MODULE_OFFLINE,
    BT_MODULE_READY,
    BT_MODULE_SPP_CONNECTION,
} bt_state_t;

typedef enum {
    BT_CMD_STATUS_CHECK,
    BT_CMD_ECHO_OFF,
    BT_CMD_GET_DEVICES,
    BT_CMD_WIRTE_S_REGISTER,
    BT_CMD_GET_FRIENDLY_NAME,
    BT_CMD_SPP_START,
    BT_CMD_SPP_STOP,
    BT_CMD_WRITE_SREG_DISCOVERABLE,
    BT_CMD_WRITE_SREG_CONNECTABLE,
    BT_CMD_LAST,
} bt_cmd_type_t;

typedef union {
    uint8_t param;
    uint8_t bt_address[BT_ADDRESS_STR_LENGTH];
} bt_cmd_params_t;

typedef struct {
    bt_cmd_type_t type;
    bt_cmd_params_t params;
} bt_cmd_t;

typedef struct {
    uint8_t bt_address[BT_ADDRESS_STR_LENGTH];
    uint8_t *data;
    uint8_t data_len;
} bt_packet_t;

typedef enum {
    BT_STATUS_OK,
    BT_STATUS_CONNECTED,
    BT_STATUS_RING,
    BT_STATUS_DATA,
    BT_STATUS_ERROR = 0x10,
    BT_STATUS_NO_CARRIER,
} bt_status_t;

typedef struct response_queue {
    uint8_t data[RESPONSE_DATA_LENGTH];
    struct response_queue *next;
} response_queue_t;

typedef void (*msg_receive_cb)(bool status, bt_packet_t *packet);
typedef void (*cmd_response_cb)(bt_status_t status, response_queue_t *resp);
typedef void (*state_cb)(bt_state_t state);

void BT740_init(void);
void BT740_sendCmd(bt_cmd_t *cmd, cmd_response_cb cb);
void BT740_register_for_messages(msg_receive_cb cb);
void BT740_send_message(bt_packet_t *packet);

void BT740_register_for_state(state_cb cb);

#endif // BT740_H
