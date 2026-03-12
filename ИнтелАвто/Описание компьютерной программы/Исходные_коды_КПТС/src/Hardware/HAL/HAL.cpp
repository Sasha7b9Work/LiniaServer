// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "Hardware/HAL/HAL.h"
#include <stm32f4xx_hal.h>


/*
*   –есурсы
*   TIM4 -  лавиатура
*   SPI1 - —в€зь с платой управлени€
*/


#ifdef PANEL
PCD_HandleTypeDef hPCD;
#endif


static void SystemClock_Config(void);


void HAL::Init()
{
    HAL_Init();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* System interrupt init*/
    /* MemoryManagement_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
    /* BusFault_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
    /* UsageFault_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
    /* SVCall_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
    /* DebugMonitor_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

#ifndef WIN32

//    __USB_OTG_FS_CLK_ENABLE();
    __USB_OTG_HS_CLK_ENABLE();

    __SYSCFG_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

#endif

    //    GPIO_InitTypeDef isGPIO =
    //    {
    //        GPIO_PIN_11 | GPIO_PIN_12,
    //        GPIO_MODE_AF_PP,
    //        GPIO_NOPULL,
    //        GPIO_SPEED_FREQ_VERY_HIGH,
    //        GPIO_AF10_OTG_FS
    //    };
    //
    //    _HAL_GPIO_Init(GPIOA, &isGPIO);

    //    isGPIO.Pin = GPIO_PIN_9;
    //    isGPIO.Mode = GPIO_MODE_INPUT;
    //    isGPIO.Pull = GPIO_NOPULL;
    //
    //    _HAL_GPIO_Init(GPIOA, &isGPIO);

//    HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);

//    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);

    SystemClock_Config();

#ifdef PANEL

    HAL_SPI1::Init();
    HAL_BUS_DISPLAY::Init();
#endif

    HAL_TIM::Init();
}


void HAL::DeInit()
{
    HAL_DeInit();
}


static void SystemClock_Config()
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
   * device is clocked below the maximum system frequency, to update the
   * voltage scaling value regarding system frequency refer to product
   * datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Configure RCC Oscillators: All parameters can be changed according to
   * userТs needs */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* RCC Clocks: All parameters can be changed according to userТs needs */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}


void HAL::ERROR_HANDLER()
{
    // \todo «десь должен был сброс
//    *((int *)((void *)0)) = 0;
}
