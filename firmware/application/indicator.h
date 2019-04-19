#ifndef __INDICATOR_H__
#define __INDICATOR_H__

enum{
    INDICATOR_ON,
    INDICATOR_OFF,
    INDICATOR_BLINK_SLOW,
    INDICATOR_BLINK_FAST,
};

enum{
    INDICATOR_0,
};

extern void indicator_set(int id, int state);
extern void indicator_flash_set(int id, int num);

static inline void indicator_turn_on(int id)
{
    indicator_set(id, INDICATOR_ON);
}

static inline void indicator_turn_off(int id)
{
    indicator_set(id, INDICATOR_OFF);
}

static inline void indicator_blink_slow(int id)
{
    indicator_set(id, INDICATOR_BLINK_SLOW);
}

static inline void indicator_blink_fast(int id)
{
    indicator_set(id, INDICATOR_BLINK_FAST);
}

#endif
