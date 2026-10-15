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
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_SAMPLES 20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// --- CẤU HÌNH THANG ĐO CẢM BIẾN ÁP SUẤT ---
#define SENSOR_MAX_VOLTAGE 3.3f   // Điện áp ngõ ra lớn nhất vào chân PA0 (V)
#define SENSOR_MAX_PRESSURE 10.0f // Thang đo áp suất tối đa của cảm biến (Bar)
#define ADC_SAMPLE_COUNT 16       // Số lần lấy mẫu trung bình để khử nhiễu

// Hệ số quy đổi áp suất sang cột nước: 1 Bar = 1019.72 cm H2O (P = rho * g * h)
#define BAR_TO_CM_WATER 1019.72f

// Biến lưu trữ giá trị đo
volatile uint32_t g_adc_raw = 0;
volatile float g_voltage = 0.0f;
volatile float g_pressure_bar = 0.0f;
volatile float g_pressure_kpa = 0.0f;
volatile float g_water_level_cm = 0.0f; // Mức nước tính bằng centimet (cm)

char display_line[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Hàm đọc giá trị ADC trung bình để kết quả không bị rung lắc
uint32_t Read_ADC_Filtered(void) {
  uint32_t sum = 0;
  for (int i = 0; i < ADC_SAMPLE_COUNT; i++) {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
      sum += HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    HAL_Delay(2);
  }
  return (sum / ADC_SAMPLE_COUNT);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // 1. Hiệu chuẩn ADC1 trên STM32F1
  HAL_ADCEx_Calibration_Start(&hadc1);

  // 2. Khởi động màn hình OLED SSD1306
  HAL_Delay(100);             // Đợi màn hình nạp tụ nguồn ổn định
  ssd1306_Init();             // Gửi tập lệnh khởi tạo OLED
  ssd1306_Fill(Black);        // Xóa sạch bộ đệm
  ssd1306_SetCursor(10, 20);
  ssd1306_WriteString("SYSTEM READY", Font_11x18, White);
  ssd1306_UpdateScreen();     // Xuất dữ liệu lên màn hình
  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // --- BƯỚC 1: ĐỌC DỮ LIỆU TỪ CẢM BIẾN ---
    g_adc_raw = Read_ADC_Filtered();

    // --- BƯỚC 2: TÍNH ĐIỆN ÁP, ÁP SUẤT & MỨC NƯỚC (CM) ---
    // Điện áp thực tế tại chân PA0 (0V - 3.3V)
    g_voltage = ((float)g_adc_raw / 4095.0f) * 3.3f;

    // Quy đổi ra áp suất theo đơn vị Bar
    g_pressure_bar = (g_voltage / SENSOR_MAX_VOLTAGE) * SENSOR_MAX_PRESSURE;
    if (g_pressure_bar < 0.0f) {
      g_pressure_bar = 0.0f;
    }

    // Quy đổi thêm đơn vị kPa (1 Bar = 100 kPa)
    g_pressure_kpa = g_pressure_bar * 100.0f;

    // Quy đổi áp suất thủy tĩnh sang chiều cao cột nước (cm): h = P / (rho * g)
    g_water_level_cm = g_pressure_bar * BAR_TO_CM_WATER;

    // --- BƯỚC 3: HIỂN THỊ LÊN MÀN HÌNH OLED ---
    ssd1306_Fill(Black); // Xóa bộ đệm màn hình

    // Dòng 1: Tiêu đề giao diện
    ssd1306_SetCursor(25, 2);
    ssd1306_WriteString("WATER LEVEL", Font_7x10, White);

    // Kẻ một đường ngang phân cách tiêu đề
    ssd1306_DrawLine(0, 14, 127, 14, White);

    // Dòng 2: Hiển thị giá trị mức nước cỡ chữ lớn (Font 11x18)
    sprintf(display_line, "%.1f cm", g_water_level_cm);
    ssd1306_SetCursor(20, 18);
    ssd1306_WriteString(display_line, Font_11x18, White);

    // Dòng 3: Hiển thị áp suất tương ứng
    sprintf(display_line, "P:  %.2f Bar", g_pressure_bar);
    ssd1306_SetCursor(5, 40);
    ssd1306_WriteString(display_line, Font_7x10, White);

    // Dòng 4: Hiển thị điện áp ngõ vào tại chân PA0
    sprintf(display_line, "Vin: %.2f V", g_voltage);
    ssd1306_SetCursor(5, 52);
    ssd1306_WriteString(display_line, Font_7x10, White);

    // LỆNH QUAN TRỌNG: Cập nhật toàn bộ khung hình ra màn hình vật lý
    ssd1306_UpdateScreen();

    HAL_Delay(300); // Tốc độ làm tươi hiển thị: ~3 lần/giây
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
