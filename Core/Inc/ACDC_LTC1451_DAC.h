/**
 * @file ACDC_LTC1451_DAC.h
 * @author Devin Marx
 * @brief Header file for the LTC1451 12-bit DAC
 * @version 0.1
 * @date 2024-03-21
 * 
 * @copyright Copyright (c) 2023-2024
 */

#ifndef __ACDC_LTC1451_DAC_H
#define __ACDC_LTC1451_DAC_H

#include "stm32f1xx.h"
#include "ACDC_stdint.h"

typedef struct {
    SPI_TypeDef *SPIx;          /**< SPI Peripheral to be used for the LTC1451 DAC */
    GPIO_TypeDef *GPIOx_CS;     /**< GPIO port for SPIx's CS                       */
    uint16_t GPIO_PIN_CS;       /**< GPIO pin for SPI'x CS                         */
} LTC1451_t;

/// @brief Initiliazes SPIx and the external LTC1451 DAC. Also sets up the sofware CS pin for SPIx
/// @param SPIx SPI Peripheral (Ex. SPI1 or SPI2)
/// @param GPIOx GPIO Port for the chip select pin (Ex. GPIOA, GPIOB, ...)
/// @param GPIO_PIN Desired chip select pin on port GPIOx (Ex. GPIO_PIN_0, GPIO_PIN_1, ...)
/// @return Struct containing all necessary data for the DAC
LTC1451_t LTCDAC_InitCS(SPI_TypeDef *SPIx, GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN);

/// @brief Sets the DAC's ouptut voltage (0-4.095v)
/// @param LTC_DAC Struct containing configuration data for the DAC
/// @param outputVal Voltage to set on the output pin of the DAC
void LTCDAC_SetOutputCS(LTC1451_t LTC_DAC, uint16_t outputVal);

#endif