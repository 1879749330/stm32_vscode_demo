#include "key.h"

// 内部状态变量
static KeyState_t g_keyState = KEY_STATE_IDLE;
static uint32_t g_keyTimer = 0;
static uint8_t g_clickCount = 0;

/**
 * @brief  按键初始化 PA3
 * @param  无
 * @retval 无
 */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 开启GPIO时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 配置PA3为输入上拉
    GPIO_InitStruct.Pin = KEY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 上拉输入，按键另一端接GND
    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStruct);

    // 初始化状态变量
    g_keyState = KEY_STATE_IDLE;
    g_keyTimer = 0;
    g_clickCount = 0;
}

/**
 * @brief  获取按键当前电平
 * @param  无
 * @retval 1:释放 0:按下
 */
static uint8_t Key_GetLevel(void)
{
    return HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN);
}

/**
 * @brief  按键状态机处理，需要在定时器中断中每隔1ms调用一次，或者在main循环中保证1ms调用一次
 * @param  无
 * @retval 无
 */
void Key_Tick(void)
{
    uint8_t currentLevel = Key_GetLevel();

    switch(g_keyState)
    {
        case KEY_STATE_IDLE:
            // 检测到按下
            if(currentLevel == KEY_PRESSED_LEVEL)
            {
                g_keyState = KEY_STATE_DEBOUNCE;
                g_keyTimer = 0;
            }
            break;

        case KEY_STATE_DEBOUNCE:
            g_keyTimer++;
            // 消抖时间到
            if(g_keyTimer >= KEY_DEBOUNCE_MS)
            {
                // 确实按下了
                if(currentLevel == KEY_PRESSED_LEVEL)
                {
                    g_keyState = KEY_STATE_PRESSED;
                    g_keyTimer = 0;
                }
                else
                {
                    // 抖动，回到空闲
                    g_keyState = KEY_STATE_IDLE;
                }
            }
            break;

        case KEY_STATE_PRESSED:
            // 检查是否释放
            if(currentLevel == KEY_RELEASED_LEVEL)
            {
                // 松开了，之前是短按，等待第二次点击判断是否双击
                g_keyState = KEY_STATE_WAIT_SECOND;
                g_keyTimer = 0;
                g_clickCount = 1;
            }
            else
            {
                // 还按着，检查是否长按
                g_keyTimer++;
                if(g_keyTimer >= KEY_LONG_PRESS_MS)
                {
                    g_keyState = KEY_STATE_IDLE;
                    // 这里不直接返回事件，留给Key_Scan获取
                    // 长按事件触发
                }
            }
            break;

        case KEY_STATE_WAIT_RELEASE:
            // 长按后等待释放
            if(currentLevel == KEY_RELEASED_LEVEL)
            {
                g_keyState = KEY_STATE_IDLE;
            }
            break;

        case KEY_STATE_WAIT_SECOND:
            g_keyTimer++;
            // 检测到第二次按下
            if(currentLevel == KEY_PRESSED_LEVEL)
            {
                g_clickCount = 2;
                g_keyState = KEY_STATE_DEBOUNCE;
                g_keyTimer = 0;
            }
            // 超时，判定为单击
            else if(g_keyTimer >= KEY_DOUBLE_CLICK_MS)
            {
                g_keyState = KEY_STATE_IDLE;
                // 单击事件触发
            }
            break;

        default:
            g_keyState = KEY_STATE_IDLE;
            break;
    }
}

/**
 * @brief  获取按键事件，调用此函数获取当前事件
 * @param  无
 * @retval KeyEvent_t 按键事件
 */
KeyEvent_t Key_Scan(void)
{
    KeyEvent_t event = KEY_EVENT_NONE;

    // 需要检查是否有长按事件触发
    if(g_keyState == KEY_STATE_IDLE)
    {
        // 如果是长按后回到空闲，检查是否触发了长按
        if(g_keyTimer >= KEY_LONG_PRESS_MS && Key_GetLevel() == KEY_RELEASED_LEVEL)
        {
            event = KEY_EVENT_LONG_PRESS;
            g_keyTimer = 0;
        }
        else if(g_clickCount == 1 && g_keyTimer >= KEY_DOUBLE_CLICK_MS)
        {
            event = KEY_EVENT_CLICK;
            g_clickCount = 0;
            g_keyTimer = 0;
        }
        else if(g_clickCount == 2)
        {
            event = KEY_EVENT_DOUBLE_CLICK;
            g_clickCount = 0;
            g_keyTimer = 0;
        }
    }

    return event;
}
