/**
 * @file ACDC_CLOCK.c
 * @author Devin Marx
 * @brief Implentation of clock configuration functions for STM32F103xB MCU
 * @version 0.1
 * @date 2023-11-3
 * 
 * @copyright Copyright (c) 2023-2024
 */

#include "ACDC_CLOCK.h"
#include "ACDC_TIMER.h"
#include "ACDC_GPIO.h"

#define MAX_MCO_CLK_SPEED 50000000

static SystemClockSpeed currentSCS;     /**< Current System Clock Speed */
static APB_Prescaler APB1_Prescaler;    /**< Current APB1 Prescaler     */
static APB_Prescaler APB2_Prescaler;    /**< Current APB2 Prescaler     */

#pragma region PRIVATE_FUNCTION_PROTOTYPES
/// @brief Disables the HSI and enables the PLL from the SysClock Mux
static void DisableHSI_EnablePLL(void);

/// @brief Enables the HSI and disables the PLL from the SysClock Mux
static void EnableHSI_DisablePLL(void);

/// @brief Sets the Flash memory speed on the CPU {See RM-58, RM-59, RM-60}
/// @param SCS_x Desired Clock Speed of the System
/// @param RCC_CFGR_HPRE_x AHB Prescaler Value
static void SetFlashMemorySpeed(SystemClockSpeed SCS_x, uint32_t RCC_CFGR_HPRE_x);

/// @brief Converts the APB_Prescaler APB_DIV_x into its divisor value (Ex. APB_DIV_8 -> 8)
/// @param APB_DIV_x Clock Prescaler Divider (Ex. PRE_DIV_1, PRE_DIV_2, ...)
/// @return Integer divisor value of the prescaler
static int APBxPrescalerToDivisor(APB_Prescaler APB_DIV_x);
#pragma endregion

#pragma region PUBLIC_FUNCTIONS
void CLOCK_SetSystemClockSpeed(SystemClockSpeed SCS_x){

    RCC->CFGR |= RCC_CFGR_PLLSRC;   // Use HSE as PLL Source,
    EnableHSI_DisablePLL();         // Disables the PLL

    // Clears HSE DIV into PLL Source Mux, Clear PLL Mull 
    RCC->CFGR &= ~(RCC_CFGR_PLLXTPRE_Msk | RCC_CFGR_PLLMULL_Msk | RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk);

    switch (SCS_x){
        case SCS_16MHz: //PLLMULL2
        case SCS_24MHz: //PLLMULL3
        case SCS_32MHz: //PLLMULL4
        case SCS_40MHz: //PLLMULL5
        case SCS_48MHz: //PLLMULL6
        case SCS_56MHz: //PLLMULL7
        case SCS_64MHz: //PLLMULL8
        case SCS_72MHz: //PLLMULL9
            SetFlashMemorySpeed(SCS_x, RCC_CFGR_HPRE_DIV1);
            RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                            // AHB Prescaler DIV1
            RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE;                         // HSE No DIV into PLL Src Mux
            RCC->CFGR |= (SCS_x / 8000000 - 2) << RCC_CFGR_PLLMULL_Pos; // Converts SCS into the PLL Multiplier
        break;

        case SCS_8MHz:  //PLLMULL2
        case SCS_12MHz: //PLLMULL3
        case SCS_20MHz: //PLLMULL5
        case SCS_28MHz: //PLLMULL7
        case SCS_36MHz: //PLLMULL9
        case SCS_44MHz: //PLLMULL11
        case SCS_52MHz: //PLLMULL13
        case SCS_60MHz: //PLLMULL15
            SetFlashMemorySpeed(SCS_x, RCC_CFGR_HPRE_DIV1);
            RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                            // AHB Prescaler DIV1
            RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_DIV2;                    // HSE DIV2 into PLL Src Mux
            RCC->CFGR |= (SCS_x / 4000000 - 2) << RCC_CFGR_PLLMULL_Pos; // Converts SCS into the PLL Multiplier 
        break;

        case SCS_4MHz:  //PLLMULL2
        case SCS_6MHz:  //PLLMULL3
        case SCS_10MHz: //PLLMULL5
        case SCS_14MHz: //PLLMULL7
        case SCS_18MHz: //PLLMULL9
        case SCS_22MHz: //PLLMULL11     
        case SCS_26MHz: //PLLMULL13
        case SCS_30MHz: //PLLMULL15 
            SetFlashMemorySpeed(SCS_x, RCC_CFGR_HPRE_DIV2);
            RCC->CFGR |= RCC_CFGR_HPRE_DIV2;                            // AHB Prescaler DIV2
            RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_DIV2;                    // HSE DIV2 into PLL Src Mux
            RCC->CFGR |= (SCS_x / 2000000 - 2) << RCC_CFGR_PLLMULL_Pos; // Converts SCS into the PLL Multiplier
        break;

        case SCS_2MHz:  //PLLMULL2
        case SCS_3MHz:  //PLLMULL3
        case SCS_5MHz:  //PLLMULL5
        case SCS_7MHz:  //PLLMULL7
        case SCS_9MHz:  //PLLMULL9
        case SCS_11MHz: //PLLMULL11
        case SCS_13MHz: //PLLMULL13
        case SCS_15MHz: //PLLMULL15
            SetFlashMemorySpeed(SCS_x, RCC_CFGR_HPRE_DIV4);
            RCC->CFGR |= RCC_CFGR_HPRE_DIV4;                            // AHB Prescaler DIV4
            RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_DIV2;                    // HSE DIV2 into PLL Src Mux
            RCC->CFGR |= (SCS_x / 1000000 - 2) << RCC_CFGR_PLLMULL_Pos; // Converts SCS into the PLL Multiplier
        break;

        case SCS_1MHz:  //PLLMULL2  // Somehow gives 8MHz
            SetFlashMemorySpeed(SCS_x, RCC_CFGR_HPRE_DIV8);
            RCC->CFGR |= RCC_CFGR_HPRE_DIV8;                            // AHB Prescaler DIV8
            RCC->CFGR |= RCC_CFGR_PLLXTPRE_HSE_DIV2;                    // HSE DIV2 into PLL Src Mux
            RCC->CFGR |= (SCS_x / 500000  - 2) << RCC_CFGR_PLLMULL_Pos; // Converts SCS into the PLL Multiplier
        break;

        default:
            break;
    }

    APB_Prescaler prescaler = SCS_x > SCS_36MHz ? APB_DIV_2 : APB_DIV_1;
    CLOCK_SetAPB1Prescaler(prescaler);   // Sets the APB1 Prescaler based on SCS (Max Speed is 36Mhz)

    DisableHSI_EnablePLL(); // Enables the PLL
    TIMER_Init(SCS_x);      // Sets SysTick's Clock Speed to SCS_x (used for Millis and Delay)
    currentSCS = SCS_x;     // Set currentSCS to SCS_x when clocks are done configuring
}

SystemClockSpeed CLOCK_GetSystemClockSpeed(void){
    return currentSCS;
}

SystemClockSpeed CLOCK_GetAPB1ClockSpeed(void){
    return CLOCK_GetSystemClockSpeed() / APBxPrescalerToDivisor(APB1_Prescaler);
}

SystemClockSpeed CLOCK_GetAPB2ClockSpeed(void){
    return CLOCK_GetSystemClockSpeed() / APBxPrescalerToDivisor(APB2_Prescaler);
}

SystemClockSpeed CLOCK_GetAPB1TimerClockSpeed(void){
    return CLOCK_GetAPB1ClockSpeed() * 2;
}


void CLOCK_SetMcoOutput(MicroClockOutput MCO_x){

    GPIO_PinDirection(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_SPEED_50MHz, GPIO_CNF_OUTPUT_AF_PUSH_PULL);

    int regVal = RCC->CFGR & ~(RCC_CFGR_MCO_Msk); // Clears the register

    if(currentSCS > MAX_MCO_CLK_SPEED)
        RCC->CFGR = regVal | RCC_CFGR_MCO_PLLCLK_DIV2;  // Sets the MCO output to 1/2 of PLL Clock
    else
        RCC->CFGR = regVal | MCO_x;                     // Sets the Desired MCO Bits
}

void CLOCK_SetADCPrescaler(ADC_Prescaler ADC_DIV_x){   
    MODIFY_REG(RCC->CFGR, RCC_CFGR_ADCPRE_Msk, ADC_DIV_x);  // Modify the register to place the new ADC Divider
}

void CLOCK_SetAPB1Prescaler(APB_Prescaler APB_DIV_x){
    uint32_t setMask = (APB_DIV_x & 0b111) << RCC_CFGR_PPRE1_Pos; // Create the mask to set the APB Divider
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1_Msk, setMask);           // Modify the Register to to place the new APB Divider
    APB1_Prescaler = APB_DIV_x;                                   // Set the Private variable
}

void CLOCK_SetAPB2Prescaler(APB_Prescaler APB_DIV_x){   
    uint32_t setMask = (APB_DIV_x & 0b111) << RCC_CFGR_PPRE2_Pos; // Create the mask to set the APB Divider
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2_Msk, setMask);           // Modify the Register to to place the new APB Divider
    APB2_Prescaler = APB_DIV_x;                                   // Set the Private variable
}
#pragma endregion

#pragma region PRIVATE_FUNCTIONS
static void DisableHSI_EnablePLL(void){
    RCC->CR |= RCC_CR_PLLON;            // Turn on the PLL Clock
    while(!(RCC->CR & RCC_CR_PLLRDY)){} // Wait until the PLL oscillator is ready

    RCC->CFGR &= ~RCC_CFGR_SW_Msk;      // Clear the SW bits
    RCC->CFGR |= RCC_CFGR_SW_PLL;       // Set PLL as SysClock

    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL)){}    // Wait until PLL is the SysClk
}

static void EnableHSI_DisablePLL(void){
    RCC->CR |= RCC_CR_HSEON;            // Turn on the HSE Clock
    while(!(RCC->CR & RCC_CR_HSERDY)){} // Wait until HSE oscillator is ready

    RCC->CFGR &= ~RCC_CFGR_SW_Msk;      // Clear the SW bits
    RCC->CFGR |= RCC_CFGR_SW_HSE;       // Set HSE as SysClock

    while(!(RCC->CFGR & RCC_CFGR_SWS_HSE)){}    // Wait until HSE is the SysClk

    RCC->CR &= ~RCC_CR_PLLON;           // Disable the PLL
}

static void SetFlashMemorySpeed(SystemClockSpeed SCS_x, uint32_t RCC_CFGR_HPRE_x){
    if(RCC_CFGR_HPRE_x == RCC_CFGR_HPRE_DIV1){              // AHB Prescaler is 1
        CLEAR_BIT(FLASH->ACR, FLASH_ACR_PRFTBE);            // Disable the prefetch buffer
        while((READ_BIT(FLASH->ACR, FLASH_ACR_PRFTBS))){}   // Wait until the buffer is disabled
    }  
    else{                                                   // AHB Prescaler is 2 or 3
        SET_BIT(FLASH->ACR, FLASH_ACR_PRFTBE);              // Enable the prefetch buffer
        while((READ_BIT(FLASH->ACR, FLASH_ACR_PRFTBS))){}   // Wait until the buffer is enabled
    }

    uint32_t flashLatency;  //Desired flash latency {See RM-58}
    if(SCS_x <= SCS_24MHz)
        flashLatency = FLASH_ACR_LATENCY_0;
    else if(SCS_x <= SCS_48MHz)
        flashLatency = FLASH_ACR_LATENCY_1;
    else
        flashLatency = FLASH_ACR_LATENCY_2;

    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY_Msk, flashLatency);
}

static int APBxPrescalerToDivisor(APB_Prescaler APB_DIV_x){
    switch(APB_DIV_x){
        case APB_DIV_16:
            return 16;
        case APB_DIV_8:
            return 8;
        case APB_DIV_4:
            return 4;
        case APB_DIV_2:
            return 2;
        default:        // APB_DIV_1
            return 1;
    }
}
#pragma endregion