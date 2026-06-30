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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <math.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"


void SystemClock_Config(void);
void I2C_Read(uint8_t address, uint8_t reg, uint8_t nb, uint8_t *data);

#define PIN_SCL 10
#define PIN_SDA 11
#define SHIFT_AMOUNT 8
#define SHIFT_MASK ((1 << SHIFT_AMOUNT) - 1)

uint8_t axes;

int main(void)
{
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  // low level access to GPIO
  // we init the pin by direct manipulation of registers
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; 	// enable the clock to GPIOC
  GPIOC->MODER = (5 << 16); // 0x28000;

  // middle level access to GPIO (via the peripheral library)
  LL_GPIO_InitTypeDef  GPIO_InitStructure;

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
  uint8_t data[3];
  int8_t x,y,z;
  int8_t Ar;

  char string[100];

  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // enable the clock to GPIOB
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // enable the clock to I2C2

  //Setting I2C pins
  GPIOB->MODER |= (LL_GPIO_MODE_ALTERNATE << PIN_SCL*2) | (LL_GPIO_MODE_ALTERNATE << PIN_SDA*2);
  GPIOB->AFR[1] |= (LL_GPIO_AF_1 << (PIN_SCL-8)*4) | (LL_GPIO_AF_1 << (PIN_SDA-8)*4);
  GPIOB->OTYPER |= (LL_GPIO_OUTPUT_OPENDRAIN << PIN_SCL) | (LL_GPIO_OUTPUT_OPENDRAIN << PIN_SDA);
  GPIOB->OSPEEDR |= (LL_GPIO_SPEED_FREQ_HIGH << PIN_SCL*2) | (LL_GPIO_SPEED_FREQ_HIGH << PIN_SDA*2);
  GPIOB->PUPDR |= (LL_GPIO_PULL_UP << PIN_SCL*2) | (LL_GPIO_PULL_UP << PIN_SDA*2);

  //Setting I2C timmings
  I2C2->CR1 &= ~I2C_CR1_PE;
  I2C2->TIMINGR = (0x13) | (0xF << 8) | (0x2 << 16) | (0x4 << 20) | (1 << 28);
  I2C2->CR1 |= I2C_CR1_PE;

  //data = malloc(3*sizeof(uint8_t));

  /*I2C_Read(address, (uint8_t)0x4, 3, data);


  x = (int8_t)data[0] >> 2;
  y = (int8_t)data[1] >> 2;
  z = (int8_t)data[2] >> 2;

  sprintf(string, "x = %d\n\n y = %d\n\n z = %d\n\n", x, y, z);*/


  while (1){

    I2C_Read(address, (uint8_t)0x4, 3, data);
    x = (int8_t)data[0] / 4;
    y = (int8_t)data[1] / 4;
    z = (int8_t)data[2] / 4;

    switch(axes){
      case 'x':
        Ar = (int8_t)(atan(x/sqrt(pow(y,2) + pow(z, 2)))*180/M_PI);
        break;
      case 'y':
        Ar = (int8_t)(atan(y/sqrt(pow(x,2) + pow(z, 2)))*180/M_PI);
        break;
      case 'z':
        Ar = (int8_t)(atan(sqrt(pow(x,2) + pow(y, 2))/z)*180/M_PI);
        break;
      default:
        sprintf(string, "Choose axes\n\r");
        CDC_Transmit_FS((uint8_t*)string, strlen(string));
        HAL_Delay(1000);
        continue;
        break;
    }
    /*float Rx = atan(x/sqrt(pow(y,2) + pow(z, 2)))*180/M_PI;
    float Ry = atan(y/sqrt(pow(x,2) + pow(z, 2)))*180/M_PI;
    float Rz = atan(sqrt(pow(x,2) + pow(y, 2))/z)*180/M_PI;
    Arx = (int16_t) (Rx * 100) % 100;*/


    /*Arx = (int8_t)(atan(x/sqrt(pow(y,2) + pow(z, 2)))*180/M_PI);
    Ary = (int8_t)(atan(y/sqrt(pow(x,2) + pow(z, 2)))*180/M_PI);
    Arz = (int8_t)(atan(sqrt(pow(x,2) + pow(y, 2))/z)*180/M_PI);*/
    sprintf(string, "%c angle = %d\n\r", axes, Ar);
    //sprintf(string, "x = %d\n\r", Arx);
    //sprintf(string, "x = %d\n\r y = %d\n\r z = %d\n\r", Arx, Ary, Arz);
    //sprintf(string, "x = %d.%d\n\r", (uint8_t)Rx, Arx);
    CDC_Transmit_FS((uint8_t*)string, strlen(string));
    HAL_Delay(1000);
  }



  /*I2C2->CR2 =  (address << 1) | (0b1 << 16) | I2C_CR2_START;
   *
   *  while((I2C2->ISR & I2C_ISR_TXIS) == 0);
   *
   *  I2C2->TXDR = (uint8_t)0x0;
   *
   *  while(!(I2C2->ISR & I2C_ISR_TC));
   *
   *
   *  I2C2->CR2 = (address << 1) | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND | (0b1 << 16) | I2C_CR2_START;
   *
   *  while((I2C2->ISR & I2C_ISR_RXNE) == 0);*/

  //sprintf(pointer, "%d ", (uint8_t)data);

  /*sprintf(pointer, "%d ", I2C2->RXDR);
   *  while (1){
   *    CDC_Transmit_FS(pointer, strlen(pointer));
   *    HAL_Delay(1000);
}*/


  //if(data == 0xDD) GPIOC->ODR ^= (1 << 8);

  //while(!((I2C2->ISR & I2C_ISR_STOPF) == I2C_ISR_STOPF));

  //I2C2->ICR |= I2C_ICR_STOPCF;
  //I2C2->CR2 = 0x0;



  //sprintf(pointer, "%d ", (uint16_t)data);

}

void I2C_Write(){

}

void I2C_Read(uint8_t address, uint8_t reg, uint8_t nb, uint8_t *data){

  I2C2->CR2 = (address << 1) | (1 << 16) | I2C_CR2_START;

  while((I2C2->ISR & I2C_ISR_TXIS) == 0);

  I2C2->TXDR = reg;

  while(!(I2C2->ISR & I2C_ISR_TC));

  I2C2->CR2 = (address << 1) | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND | (nb << 16) | I2C_CR2_START;

  for(uint8_t i = 0; i < nb; i++){
    while((I2C2->ISR & I2C_ISR_RXNE) == 0);
    data[i] = I2C2->RXDR;
  }

  while(!((I2C2->ISR & I2C_ISR_STOPF) == I2C_ISR_STOPF));

  I2C2->ICR |= I2C_ICR_STOPCF;
  I2C2->CR2 = 0x0;
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
   *     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

