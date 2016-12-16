#ifndef BT740_H_
#define BT740_H_

typedef enum {
    BT_CMD_UNKNOWN,
    BT_CMD_STATUS_CHECK,
    BT_CMD_WIRTE_S_REGISTER,
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

typedef void (*message_cb)(uint8_t *data, uint8_t data_len);

void BT740_init(void);
void BT740_sendCmd(bt_cmd_t *cmd, bt_cmd_response_t *response);
void BT740_register_for_packets(message_cb cb);
void BT740_send_packet(uint8_t *data, uint8_t data_len);

#endif // BT740_H
