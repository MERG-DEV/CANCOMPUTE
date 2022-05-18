#ifndef COMPUTE_ACTIONS_H
#define COMPUTE_ACTIONS_H

#include "happeningsActions.h"

#define EVENT_NO(arg)       ((arg)&0x7F)
#define EVENT_STATE(arg)    ((arg)&0x80)
#define EVENT_ON            0x80

extern void initActions(void);

#endif