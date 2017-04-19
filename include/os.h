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

#define APP_SHORT_PRESS_NOTIF             (1 << 1)
#define APP_LONG_PRESS_NOTIF              (1 << 2)

#define MES_READ_O2CELLS_NOTIF            (1 << 10)

#endif /* OS_H */
