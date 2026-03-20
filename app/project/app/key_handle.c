#include "stdbool.h"
#include "key_handle.h"

KEY_DEFINE(key1,    GPIOB, GPIO_PINS_0, 0);
KEY_DEFINE(key2,    GPIOB, GPIO_PINS_1, 0);
KEY_DEFINE(key3,    GPIOB, GPIO_PINS_2, 0);
KEY_DEFINE(key4,    GPIOB, GPIO_PINS_12, 0);
KEY_DEFINE(key5,    GPIOB, GPIO_PINS_13, 0);
KEY_DEFINE(key6,    GPIOC, GPIO_PINS_7,  0);
KEY_DEFINE(key7,    GPIOC, GPIO_PINS_10,  0);
KEY_DEFINE(key8,    GPIOA, GPIO_PINS_15,  0);
KEY_DEFINE(key9,    GPIOC, GPIO_PINS_8,   0);

#define KEY_COUNT 9

key_obj *keys[KEY_COUNT] = {&key1, &key2, &key3, &key4, &key5, &key6, &key7, &key8, &key9};
key_event_t key_event[KEY_COUNT];
static EVENT KEY_HANDLE_EVENT = END_EVENT;

void handle_key_events(void)
{
    static bool first_in = false;

    if (first_in == false)
    {
        first_in = true;
        key_set_scan_interval(KEY_TASK_TIME);
        key_set_timing_simple(&key1,    KEY_TASK_TIME, 800, 0, 100);
        key_set_timing_simple(&key2,    KEY_TASK_TIME, 800, 0, 0);
        key_set_timing_simple(&key3,    KEY_TASK_TIME, 800, 0, 0);
        key_set_timing_simple(&key4,    KEY_TASK_TIME, 800, 0, 0);
        key_set_timing_simple(&key5,    KEY_TASK_TIME, 800, 0, 100);
        key_set_timing_simple(&key6,    KEY_TASK_TIME, 800, 300, 0);
        key_set_timing_simple(&key7,    KEY_TASK_TIME, 800, 0, 0);
        key_set_timing_simple(&key8,    KEY_TASK_TIME, 800, 0, 0);
        key_set_timing_simple(&key9,    KEY_TASK_TIME, 800, 0, 100);
    }

    for (int i = 0; i < KEY_COUNT; i++)
    {
        key_event[i] = key_event_check(keys[i]);
    }

    if (key_event[0] != KEY_NONE)
    {
        switch (key_event[0])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS_CONTINUE:
            break;

        default:
            break;
        }
    }

    if (key_event[1] != KEY_NONE)
    {
        switch (key_event[1])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        default:
            break;
        }
    }

    if (key_event[2] != KEY_NONE)
    {
        switch (key_event[2])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        default:
            break;
        }
    }

    if (key_event[3] != KEY_NONE)
    {
        switch (key_event[3])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        default:
            break;
        }
    }

    if (key_event[4] != KEY_NONE)
    {
        switch (key_event[4])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS_CONTINUE:
            break;

        default:
            break;
        }
    }

    if (key_event[5] != KEY_NONE)
    {
        switch (key_event[5])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        case KEY_DOUBLE_CLICK:
            break;

        default:
            break;
        }
    }

    if (key_event[6] != KEY_NONE)
    {
        switch (key_event[6])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        case KEY_DOUBLE_CLICK:
            break;

        default:
            break;
        }
    }

    if (key_event[7] != KEY_NONE)
    {
        switch (key_event[7])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        case KEY_DOUBLE_CLICK:
            break;

        default:
            break;
        }
    }

    if (key_event[8] != KEY_NONE)
    {
        switch (key_event[8])
        {
        case KEY_SHORT_PRESS:
            break;

        case KEY_LONG_PRESS:
            break;

        case KEY_DOUBLE_CLICK:
            break;

        default:
            break;
        }
    }
}

static void event_handle(void)
{
}

void key_handle(void)
{
    handle_key_events();
    event_handle();
}

