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
#include "usb_device.h"
#include "usbd_cdc_if.h"


void SystemClock_Config(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  // low level access to GPIO
  // we init the pin by direct manipulation of registers
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; 	// enable the clock to GPIOC
  // (RM0091 lists this as IOPCEN, not GPIOCEN)
  GPIOC->MODER = (5 << 16); // 0x28000;

  // middle level access to GPIO (via the peripheral library)
  LL_GPIO_InitTypeDef  GPIO_InitStructure;

  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);  // Enable the GPIO LEDs clock

  // by [DocID025474 Rev 1] the red LED is hardwired to PC7
  // by default PC7 is the 7th signal of GPIO bank C
  GPIO_InitStructure.Pin = LL_GPIO_PIN_7;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_MEDIUM;
  LL_GPIO_Init(GPIOC, &GPIO_InitStructure);



  MX_USB_DEVICE_Init();

  uint8_t address = 0x0A;
  uint32_t number;
  uint8_t* pointer;

  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // enable the clock to GPIOB

  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // enable the clock to I2C2


  //MX_I2C2_Init();

  GPIOB->MODER |= (0b10 << 10*2) | (0b10 << 11*2);
  GPIOB->AFR[1] |= (0b0001 << 2*4) | (0b0001 << 3*4);
  GPIOB->OTYPER |= (0b1 << 10) | (0b1 << 11);
  GPIOB->OSPEEDR |= (0b11 << 10*2) | (0b11 << 11*2);
  GPIOB->PUPDR |= (0b01 << 10*2) | (0b01 << 11*2);

  I2C2->CR1 &= ~I2C_CR1_PE;
  I2C2->TIMINGR = (0x13) | (0xF << 8) | (0x2 << 16) | (0x4 << 20) | (1 << 28);
  I2C2->CR1 |= I2C_CR1_PE;



  I2C2->CR2 =  (address << 1) | (0b1 << 16) | I2C_CR2_START;

  while((I2C2->ISR & I2C_ISR_TXIS) == 0);

  I2C2->TXDR = (uint8_t)0x0;

  while(!(I2C2->ISR & I2C_ISR_TC)) GPIOC->ODR ^= (1 << 7);


  I2C2->CR2 = (address << 1) | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND | (0b1 << 16) | I2C_CR2_START;

  while((I2C2->ISR & I2C_ISR_RXNE) == 0);


  /*sprintf(pointer, "%d ", I2C2->RXDR);
  while (1){
    CDC_Transmit_FS(pointer, strlen(pointer));
    HAL_Delay(1000);
  }*/

  if(I2C2->RXDR == 0xDD) GPIOC->ODR ^= (1 << 8);

  while(!((I2C2->ISR & I2C_ISR_STOPF) == I2C_ISR_STOPF));

  I2C2->ICR |= I2C_ICR_STOPCF;
  I2C2->CR2 = 0x0;






  while (1){
    CDC_Transmit_FS((uint8_t *)"Hello World\n", 12);
    HAL_Delay(1000);
  }
  /*


  number = I2C2->ISR;
  sprintf(pointer, "%d ", I2C_ISR_TC);
  while (1){
    CDC_Transmit_FS(pointer, sizeof(I2C_ISR_TC));
    HAL_Delay(1000);
}









  while(I2C_ISR_TXIS);

  I2C2->TXDR = 0x0;

  while(~I2C_ISR_TC);

  I2C2->ICR |= I2C_ICR_STOPCF;
  I2C2->CR2 = 0x0;

  I2C2->CR2 |=  (address << 1) | is_read | I2C_CR2_AUTOEND | (1 << 16) | I2C_CR2_START;

  while(I2C_ISR_RXNE);

  if(I2C2->RXDR == 0xDD);

  while(~I2C_ISR_TC);

  I2C2->ICR |= I2C_ICR_STOPCF;
  I2C2->CR2 = 0x0;
*/



}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI48_Enable();

   /* Wait till HSI48 is ready */
  while(LL_RCC_HSI48_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PREDIV_DIV_1);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(48000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_HSI48);
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C2 GPIO Configuration
  PB10   ------> I2C2_SCL
  PB11   ------> I2C2_SDA
  */
  GPIO_InitStruct.Pin = I2C2_SCL_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(I2C2_SCL_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = I2C2_SDA_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(I2C2_SDA_GPIO_Port, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */

  /** I2C Initialization
  */
  LL_I2C_DisableOwnAddress2(I2C2);
  LL_I2C_DisableGeneralCall(I2C2);
  LL_I2C_EnableClockStretching(I2C2);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x20303E5D;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C2, &I2C_InitStruct);
  LL_I2C_EnableAutoEndMode(I2C2);
  LL_I2C_SetOwnAddress2(I2C2, 0, LL_I2C_OWNADDRESS2_NOMASK);
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

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
