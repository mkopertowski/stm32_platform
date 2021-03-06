/** \file os.h
 *  \brief OS helper definition file
 */
#ifndef OS_H
#define OS_H

#include <task.h>

#define OS_TASK_NOTIFY_MASK 0xFFFFFFFF

#define OS_TASK_PRIORITY              ( tskIDLE_PRIORITY + 1 )

#define OS_TASK_NOTIFY(task, value) \
        ({ \
                BaseType_t need_switch, ret; \
                ret = xTaskNotifyFromISR(task, value, eSetBits, &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

#define OS_QUEUE_PUT_FROM_ISR(queue, item) \
        ({ \
                BaseType_t need_switch, ret; \
                ret = xQueueSendToBackFromISR((queue), (item), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

#define APP_BT_MODULE_READY_NOTIF         (1 << 1)
#define APP_BT_MODULE_RESPONSE_NOTIF      (1 << 2)
#define APP_SHORT_PRESS_NOTIF             (1 << 3)
#define APP_LONG_PRESS_NOTIF              (1 << 4)
#define APP_MODULE_HITTED_NOTIF           (1 << 5)
#define APP_SPP_DATA_RECEIVED_NOTIF       (1 << 6)

#define BT740_RESPONSE_WAITING_NOTIF      (1 << 10)
#define BT740_SETUP_DONE_NOTIF            (1 << 11)
#define BT740_SEND_COMMAND_NOTIF          (1 << 12)

#define BT740_SPP_CONNECTED_NOTIF         (1 << 13)
#define BT740_SPP_NO_CARRIER_NOTIF        (1 << 14)
#define BT740_SPP_ERROR_NOTIF             (1 << 15)
#define BT740_SPP_DISCONNECT_NOTIF        (1 << 16)
#define BT740_SPP_SEND_ESCAPE_CHAR_NOTIF  (1 << 17)

#endif /* OS_H */
