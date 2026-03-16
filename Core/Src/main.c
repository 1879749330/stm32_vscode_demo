/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "key.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* 显示界面编号 */
typedef enum
{
    UI_PAGE_BOOT = 0,    // 开机欢迎界面
    UI_PAGE_MONITOR,     // 环境监测主界面（心率/血氧/温度/水位）
    UI_PAGE_INFO,        // 系统信息界面
    UI_PAGE_MAX          // 界面总数，用于循环计数
} UIPage_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static UIPage_t current_page = UI_PAGE_BOOT;  // 当前显示界面
/* 预留传感器数据变量，后续添加传感器驱动后使用 */
static uint16_t heart_rate = 0;       // 心率值（MAX30102）
static uint8_t  spo2 = 0;             // 血氧饱和度百分比（MAX30102）
static float    temperature = 0.0f;   // 温度值（DS18B20）
static uint8_t  water_level = 0;      // 水位状态（0=正常 1=报警，数字量输出）
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void UI_ShowPage(UIPage_t page);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief  根据当前界面编号刷新显示内容
 * @param  page 要显示的界面编号
 * @retval 无
 */
static void UI_ShowPage(UIPage_t page)
{
    OLED_Clear();  // 清屏后重新显示

    switch (page)
    {
        case UI_PAGE_BOOT:
            /* 第一个界面：开机欢迎画面 */
            OLED_ShowString(1, 4, "STM32");
            // 监(8) 测(9) 系(10) 统(11)
            OLED_ShowChineseChar(3, 2, 8);
            OLED_ShowChineseChar(3, 3, 9);
            OLED_ShowChineseChar(3, 4, 10);
            OLED_ShowChineseChar(3, 5, 11);
            break;

        case UI_PAGE_MONITOR:
            /* 第二个界面：四行分别显示各项监测参数 */
            // 心(0) 率(1)
            OLED_ShowChineseChar(1, 1, 0);
            OLED_ShowChineseChar(1, 2, 1);
            OLED_ShowString(1, 5, ": ---");

            // 血(2) 氧(3)
            OLED_ShowChineseChar(2, 1, 2);
            OLED_ShowChineseChar(2, 2, 3);
            OLED_ShowString(2, 5, ": --%");

            // 温(4) 度(5)
            OLED_ShowChineseChar(3, 1, 4);
            OLED_ShowChineseChar(3, 2, 5);
            OLED_ShowString(3, 5, ": --.-C");

            // 水(6) 位(7)
            OLED_ShowChineseChar(4, 1, 6);
            OLED_ShowChineseChar(4, 2, 7);
            // 正(12) 常(13)
            OLED_ShowString(4, 5, ": ");
            OLED_ShowChineseChar(4, 6, 12);
            OLED_ShowChineseChar(4, 7, 13);
            break;

        case UI_PAGE_INFO:
            /* 第三个界面：系统信息 */
            // 系(10) 统(11) 信(19) 息(20)
            OLED_ShowChineseChar(1, 1, 10);
            OLED_ShowChineseChar(1, 2, 11);
            OLED_ShowChineseChar(1, 3, 19);
            OLED_ShowChineseChar(1, 4, 20);
            OLED_ShowString(2, 1, "STM32F103C8T6");
            // 多(16) 参(17) 数(18) 监(8) 测(9)
            OLED_ShowChineseChar(3, 1, 16);
            OLED_ShowChineseChar(3, 2, 17);
            OLED_ShowChineseChar(3, 3, 18);
            OLED_ShowChineseChar(3, 4, 8);
            OLED_ShowChineseChar(3, 5, 9);
            // 校(14) 准(15) 中
            OLED_ShowChineseChar(4, 1, 14);
            OLED_ShowChineseChar(4, 2, 15);
            OLED_ShowString(4, 3, "中...");
            break;

        default:
            OLED_ShowString(1, 1, "Error:");
            OLED_ShowString(2, 1, "Unknown page");
            break;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */
    OLED_Init();         // 初始化OLED显示屏
    Key_Init();          // 初始化按键

    /* 显示第一个开机界面 */
    UI_ShowPage(current_page);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        Key_Tick();       // 按键定时扫描（需要10ms调用一次，主循环约10ms间隔也可正常工作）

        /* 获取按键事件 */
        KeyEvent_t ev = Key_Scan();
        if (ev != KEY_EVENT_NONE)
        {
            if (ev == KEY_EVENT_CLICK)
            {
                /* 单击切换到下一个界面，循环显示 */
                current_page = (UIPage_t)((current_page + 1) % UI_PAGE_MAX);
                UI_ShowPage(current_page);
            }
            /* 双击、长按可添加其他功能 */
        }

        /* 传感器数据更新代码预留位置
         * 后续添加DS18B20、MAX30102、水位传感器驱动后
         * 可在这里周期性读取数据，更新显示
         */

        HAL_Delay(10);  // 10ms扫描间隔，按键消抖效果较好
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
