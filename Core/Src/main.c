/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

xQueueHandle qh = 0;

typedef struct {
    char msg;    
} queue_item_t;

void led_task(void *param)
{
    queue_item_t rx_item;
    char key = 1;

    while (1) {
        if (key) {
            while (1) {
                // green LED for 5 sec, polling to recv msg from button
                HAL_GPIO_WritePin(Green_LED_GPIO_Port, GPIO_PIN_12, GPIO_PIN_SET);
                if (xQueueReceive(qh, &rx_item, 5000)) {
                    HAL_GPIO_WritePin(Green_LED_GPIO_Port, GPIO_PIN_12, GPIO_PIN_RESET);
                    key = 0;
                    break;
                }
                HAL_GPIO_WritePin(Green_LED_GPIO_Port, GPIO_PIN_12, GPIO_PIN_RESET);   // reset

                // red LED for 5 sec, polling to recv msg from button
                HAL_GPIO_WritePin(Red_LED_GPIO_Port, GPIO_PIN_14, GPIO_PIN_SET);
                if (xQueueReceive(qh, &rx_item, 5000)) {
                    HAL_GPIO_WritePin(Red_LED_GPIO_Port, GPIO_PIN_12, GPIO_PIN_RESET);
                    key = 0;
                    break;
                }
                HAL_GPIO_WritePin(Red_LED_GPIO_Port, GPIO_PIN_14, GPIO_PIN_RESET);     // reset
            }
        }
        else {
            while (1) {

                HAL_GPIO_TogglePin(Red_LED_GPIO_Port, GPIO_PIN_14);                   // blink
                if (xQueueReceive(qh, &rx_item, 100)) {
                    HAL_GPIO_WritePin(Red_LED_GPIO_Port, GPIO_PIN_14, GPIO_PIN_RESET);
                    key = 1;
                    break;
                }
            }
        }
    }
}

void button_task(void *param)
{
    queue_item_t tx_item;
    char prev_state = 0;
    char current_state = 0;

    while (1) {

        current_state = HAL_GPIO_ReadPin(Blue_Button_GPIO_Port, GPIO_PIN_0);

        if (current_state == 0) {
            prev_state = 0;
        }
        else if (current_state > 0 && prev_state == 0) {
            xQueueSend(qh, &tx_item, 0);
            prev_state = 1;
        }
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    qh = xQueueCreate(5, sizeof(queue_item_t));
    xTaskCreate(button_task, "detect_task", 256, NULL, 0, NULL);
    xTaskCreate(led_task, "led_task", 256, NULL, 0, NULL);

    vTaskStartScheduler();

    while (1){

    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, Green_LED_Pin|Red_LED_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : Blue_Button_Pin */
    GPIO_InitStruct.Pin = Blue_Button_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Blue_Button_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : Green_LED_Pin Red_LED_Pin */
    GPIO_InitStruct.Pin = Green_LED_Pin|Red_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        HAL_IncTick();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1);
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
