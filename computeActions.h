#ifndef COMPUTE_ACTIONS_H
#define COMPUTE_ACTIONS_H

#include "happeningsActions.h"

#define ACTION_OPCODE_NOP       0
#define ACTION_OPCODE_SEND_ON   1
#define ACTION_OPCODE_SEND_OFF  2
#define ACTION_OPCODE_DELAY     3
#define ACTION_OPCODE_SEND_CBUS 4

#define EVENT_NO(arg)       ((arg)&0x7F)
#define EVENT_STATE(arg)    ((arg)&0x80)
#define EVENT_ON            0x80

extern void initActions(void);

#endif