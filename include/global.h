/** \file global.h
 *  \brief Global definition file
 *
 *  This file should be include as first in every c-file
 */
#ifndef GLOBAL_H
#define GLOBAL_H

#define DEBUG_ON

typedef enum {
    false = 0,
    true
} bool;

#define ON true
#define OFF false

#define BT_ADDRESS_LENGTH     (12)
#define BT_ADDRESS_STR_LENGTH (BT_ADDRESS_LENGTH+1)

#endif /* GLOBAL_H */
