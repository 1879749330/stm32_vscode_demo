#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"

// 按键事件定义
typedef enum {
    KEY_EVENT_NONE = 0,      // 无事件
    KEY_EVENT_CLICK,         // 单击事件
    KEY_EVENT_DOUBLE_CLICK,  // 双击事件
    KEY_EVENT_LONG_PRESS     // 长按事件
} KeyEvent_t;

// 按键状态机状态定义
typedef enum {
    KEY_STATE_IDLE = 0,      // 空闲状态
    KEY_STATE_DEBOUNCE,      // 消抖等待
    KEY_STATE_PRESSED,       // 已按下
    KEY_STATE_WAIT_RELEASE,  // 等待释放
    KEY_STATE_WAIT_SECOND    // 等待第二次点击
} KeyState_t;

// 按键参数配置（可根据实际情况调整）
#define KEY_DEBOUNCE_MS       20      // 消抖时间(ms)
#define KEY_LONG_PRESS_MS     500     // 长按判定时间(ms)
#define KEY_DOUBLE_CLICK_MS   300     // 双击间隔时间(ms)

// 按键引脚定义 PA3
#define KEY_PORT              GPIOA
#define KEY_PIN               GPIO_PIN_3

// 按键电平定义 假设按键按下是低电平
#define KEY_PRESSED_LEVEL     0
#define KEY_RELEASED_LEVEL     1

// 函数声明
void Key_Init(void);
KeyEvent_t Key_Scan(void);
void Key_Tick(void);

#endif // __KEY_H
