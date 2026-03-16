#include "button.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdbool.h>

// 按键配置参数
#define BUTTON_ACTIVE_LEVEL   GPIO_PIN_RESET  // 按键按下时的电平（低电平有效）
#define BUTTON_DEBOUNCE_MS    50              // 消抖时间（毫秒）
#define BUTTON_LONG_MS        800             // 长按判定时间（毫秒）
#define BUTTON_DOUBLE_MS      250             // 双击间隔时间（毫秒）

// 按键状态机状态定义
typedef enum
{
    BTN_STATE_IDLE = 0,                // 空闲状态，等待按键按下
    BTN_STATE_DEBOUNCE_PRESS,          // 第一次按下消抖中
    BTN_STATE_PRESSED,                 // 已确认按下，等待释放或长按
    BTN_STATE_WAIT_DOUBLE,             // 等待第二次按下（检测双击）
    BTN_STATE_DEBOUNCE_SECOND_PRESS,   // 第二次按下消抖中
    BTN_STATE_SECOND_PRESSED,          // 第二次按下确认，等待释放
    BTN_STATE_LONG_PRESS_ACTIVE,       // 长按已触发，等待释放
} ButtonState_t;

// 按键状态机全局变量
static ButtonState_t s_state = BTN_STATE_IDLE;  // 当前状态
static uint32_t s_state_ts = 0;                 // 状态转换时间戳（HAL_GetTick）
static ButtonEvent_t s_event = BUTTON_EVENT_NONE; // 待输出的事件
static uint8_t s_click_count = 0;               // 点击计数

/**
 * @brief  读取按键当前电平
 * @retval true 按键已按下，false 按键未按下
 */
static bool Button_IsPressed(void)
{
    return (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == BUTTON_ACTIVE_LEVEL);
}

/**
 * @brief  按键初始化
 * @note   配置按键GPIO为上拉输入，初始化状态机
 * @retval 无
 */
void Button_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    // 确保GPIO端口时钟已使能（CubeMX已配置，但重复调用安全）
    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Pin = Key1_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(Key1_GPIO_Port, &gpio);

    // 初始化状态机变量
    s_state = BTN_STATE_IDLE;
    s_state_ts = HAL_GetTick();
    s_event = BUTTON_EVENT_NONE;
    s_click_count = 0;
}

/**
 * @brief  按键状态机任务处理函数
 * @note   需要定期调用（建议每10~20ms调用一次），用于消抖和事件检测
 * @retval 无
 */
void Button_Task(void)
{
    uint32_t now = HAL_GetTick();
    bool pressed = Button_IsPressed();

    switch (s_state)
    {
    case BTN_STATE_IDLE:
        // 空闲状态，检测是否有按键按下
        if (pressed)
        {
            s_state = BTN_STATE_DEBOUNCE_PRESS;
            s_state_ts = now;
        }
        break;

    case BTN_STATE_DEBOUNCE_PRESS:
        // 第一次按下消抖阶段
        if (!pressed)
        {
            // 消抖期间又松开，回到空闲状态（干扰）
            s_state = BTN_STATE_IDLE;
            break;
        }
        // 消抖时间到，确认按键按下
        if ((now - s_state_ts) >= BUTTON_DEBOUNCE_MS)
        {
            s_state = BTN_STATE_PRESSED;
            s_state_ts = now;
        }
        break;

    case BTN_STATE_PRESSED:
        // 按键已按下，等待释放或检测长按
        if (!pressed)
        {
            // 按键释放，判断是否为长按
            uint32_t press_time = now - s_state_ts;
            if (press_time >= BUTTON_LONG_MS)
            {
                // 长按事件
                s_event = BUTTON_EVENT_LONG_PRESS;
                s_state = BTN_STATE_IDLE;
            }
            else
            {
                // 短按，开始等待第二次点击（检测双击）
                s_click_count = 1;
                s_state = BTN_STATE_WAIT_DOUBLE;
                s_state_ts = now;
            }
            break;
        }

        // 按下时间超过长按阈值，触发长按事件
        if ((now - s_state_ts) >= BUTTON_LONG_MS)
        {
            s_event = BUTTON_EVENT_LONG_PRESS;
            s_state = BTN_STATE_LONG_PRESS_ACTIVE;
        }
        break;

    case BTN_STATE_LONG_PRESS_ACTIVE:
        // 长按已触发，等待按键释放
        if (!pressed)
        {
            s_state = BTN_STATE_IDLE;
        }
        break;

    case BTN_STATE_WAIT_DOUBLE:
        // 已单击一次，等待第二次点击
        if (pressed)
        {
            // 有第二次按下，进入消抖
            s_state = BTN_STATE_DEBOUNCE_SECOND_PRESS;
            s_state_ts = now;
        }
        else if ((now - s_state_ts) >= BUTTON_DOUBLE_MS)
        {
            // 双击超时，没有第二次点击，上报单击事件
            if (s_click_count == 1)
            {
                s_event = BUTTON_EVENT_SINGLE_CLICK;
            }
            s_state = BTN_STATE_IDLE;
        }
        break;

    case BTN_STATE_DEBOUNCE_SECOND_PRESS:
        // 第二次按下消抖阶段
        if (!pressed)
        {
            // 消抖期间松开，回去继续等待
            s_state = BTN_STATE_WAIT_DOUBLE;
            break;
        }
        // 消抖时间到，确认第二次按下
        if ((now - s_state_ts) >= BUTTON_DEBOUNCE_MS)
        {
            s_state = BTN_STATE_SECOND_PRESSED;
            s_state_ts = now;
        }
        break;

    case BTN_STATE_SECOND_PRESSED:
        // 第二次按下已确认，等待释放
        if (!pressed)
        {
            // 释放，触发双击事件
            s_event = BUTTON_EVENT_DOUBLE_CLICK;
            s_state = BTN_STATE_IDLE;
        }
        break;

    default:
        // 异常状态，回到空闲
        s_state = BTN_STATE_IDLE;
        break;
    }
}

/**
 * @brief  获取最新的按键事件
 * @note   获取后事件会被清除，多次调用只有第一次能获取到事件
 * @retval ButtonEvent_t 按键事件（单击、双击、长按、无事件）
 */
ButtonEvent_t Button_GetEvent(void)
{
    ButtonEvent_t ev = s_event;
    s_event = BUTTON_EVENT_NONE;
    return ev;
}
