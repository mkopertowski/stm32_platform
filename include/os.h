/** \file os.h
 *  \brief OS helper definition file
 */
#ifndef OS_H
#define OS_H

#include <task.h>

#define OS_TASK_NOTIFY_MASK 0xFFFFFFFF

#define OS_TASK_NOTIFY(task, value) \
        ({ \
                BaseType_t need_switch, ret; \
                ret = xTaskNotifyFromISR(task, value, eSetBits, &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

#endif /* OS_H */
