#include "key.h"
#include <stddef.h>

// 全局扫描间隔（默认为20ms）
static uint16_t g_scan_interval_ms = 20;

/**
 * @brief 设置扫描周期
 */
void key_set_scan_interval(uint16_t scan_interval_ms) {
    if (scan_interval_ms > 0) {
        g_scan_interval_ms = scan_interval_ms;
    }
}

/**
 * @brief 将毫秒时间转换为扫描次数
 */
static uint16_t ms_to_scan_count(uint16_t ms) {
    if (ms == 0 || g_scan_interval_ms == 0) return 0;
    return (ms + g_scan_interval_ms - 1) / g_scan_interval_ms;
}

/**
 * @brief 读取按键当前电平状态
 */
static uint8_t read_key_level(key_obj *key) {
    uint8_t gpio_level = gpio_input_data_bit_read(key->port, key->pin) ? 1 : 0;
    return (gpio_level == key->active_level) ? 1 : 0;
}

/**
 * @brief 设置按键所有时间参数
 */
void key_set_timing(key_obj *key, key_timing_config_t *timing) {
    if (key == NULL || timing == NULL) return;
    
    key->debounce_count = ms_to_scan_count(timing->debounce_ms);
    key->long_press_count = ms_to_scan_count(timing->long_press_ms);
    key->double_click_count = ms_to_scan_count(timing->double_click_interval_ms);
    key->long_press_continue_interval = ms_to_scan_count(timing->long_press_continue_interval_ms);
    
    // 确保最小值
    if (key->debounce_count == 0 && timing->debounce_ms > 0) {
        key->debounce_count = 1;
    }
}

/**
 * @brief 快速设置按键时间参数
 */
void key_set_timing_simple(key_obj *key, 
                          uint16_t debounce_ms, 
                          uint16_t long_press_ms, 
                          uint16_t double_click_interval_ms,
                          uint16_t long_press_continue_interval_ms) {
    key_timing_config_t timing = {
        .debounce_ms = debounce_ms,
        .long_press_ms = long_press_ms,
        .double_click_interval_ms = double_click_interval_ms,
        .long_press_continue_interval_ms = long_press_continue_interval_ms
    };
    
    key_set_timing(key, &timing);
}

key_event_t key_event_check(key_obj *key) {
    uint8_t current_level = read_key_level(key);
    key_event_t event = KEY_NONE;
    
    switch (key->state) {
        case STATE_RELEASED:
            if (current_level) {
                key->state = STATE_DEBOUNCE;
                key->press_count = 0;
                key->long_press_triggered = 0;
                key->long_press_continue_count = 0;
            }
            key->release_count = 0;
            break;
            
        case STATE_DEBOUNCE:
            if (current_level) {
                key->press_count++;
                if (key->press_count >= key->debounce_count) {
                    key->state = STATE_PRESSED;
                    event = KEY_PRESS;
                    key->click_count++;
                    key->press_count = 0;
                }
            } else {
                key->state = STATE_RELEASED;
            }
            break;
            
        case STATE_PRESSED:
            if (!current_level) {
                key->state = STATE_DEBOUNCE;
                key->release_count = 0;
                
                if (key->press_count < key->long_press_count) {
                    // 短按释放
                    if (key->click_count == 1) {
                        // 第一次单击，进入等待双击状态
                        key->state = STATE_WAIT_DOUBLE;
                        // 不触发事件，等待可能的第二次点击
                    } else if (key->click_count >= 2) {
                        // 第二次点击释放，触发双击事件
                        event = KEY_DOUBLE_CLICK;
                        key->click_count = 0;
                        key->state = STATE_RELEASED; // 双击完成后直接回到释放状态
                    }
                } else {
                    // 长按释放，触发短按事件（可选，或者可以触发长按释放事件）
                    event = KEY_SHORT_PRESS;
                    key->click_count = 0;
                }
                key->press_count = 0;
                key->long_press_triggered = 0;
            } else {
                key->press_count++;
                
                // 检查是否达到长按时间
                if (key->press_count >= key->long_press_count) {
                    if (!key->long_press_triggered) {
                        // 第一次达到长按阈值
                        event = KEY_LONG_PRESS;
                        key->long_press_triggered = 1;
                        key->state = STATE_LONG_PRESSED;
                        key->long_press_continue_count = 0;
                        key->click_count = 0; // 长按时重置点击计数，避免影响双击
                    }
                }
            }
            break;
            
        case STATE_LONG_PRESSED:
            if (!current_level) {
                key->state = STATE_DEBOUNCE;
                key->long_press_triggered = 0;
                key->press_count = 0;
            } else {
                // 长按持续状态
                key->long_press_continue_count++;
                
                // 检查是否达到持续触发间隔
                if (key->long_press_continue_interval > 0 && 
                    key->long_press_continue_count >= key->long_press_continue_interval) {
                    event = KEY_LONG_PRESS_CONTINUE;
                    key->long_press_continue_count = 0;
                }
            }
            break;
            
        case STATE_WAIT_DOUBLE:
            if (current_level) {
                // 第二次按下开始
                key->state = STATE_DEBOUNCE;
                key->press_count = 0;
                key->release_count = 0;
            } else {
                key->release_count++;
                if (key->release_count >= key->double_click_count) {
                    // 超时，确定为单击
                    if (key->click_count == 1) {
                        event = KEY_SHORT_PRESS;
                    }
                    key->state = STATE_RELEASED;
                    key->click_count = 0;
                }
            }
            break;
    }
    
    return event;
}

uint8_t key_is_pressed(key_obj *key) {
    return (key->state == STATE_PRESSED || key->state == STATE_LONG_PRESSED);
}

uint8_t key_is_long_pressed(key_obj *key) {
    return (key->state == STATE_LONG_PRESSED);
}