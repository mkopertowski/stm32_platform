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

typedef void (*data_receive_cb)(bool status, uint8_t *data, uint8_t data_len);
typedef void (*cmd_response_cb)(bt_status_t status, response_queue_t *resp);
typedef void (*state_cb)(bt_state_t state);

void BT740_init(void);
void BT740_sendCmd(bt_cmd_t *cmd, cmd_response_cb cb);
void BT740_response_free(response_queue_t *resp);

void BT740_register_for_spp_data(data_receive_cb cb);
void BT740_send_spp_data(uint8_t bt_address[], uint8_t *data, uint8_t data_len);

void BT740_register_for_state(state_cb cb);

#endif // BT740_H
