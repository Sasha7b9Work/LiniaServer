// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "Hardware/HAL/HAL.h"
#include "Utils/Debug_.h"
#include "FlashDrive/FlashDrive.h"
#include "VCP/VCP.h"
#include "Hardware/Beeper.h"
#include <stm32f4xx_hal.h>


#ifdef __cplusplus
extern "C" {
#endif


void NMI_Handler(void)
{

}

__attribute((noreturn)) void HardFault_Handler(void)
{
    static volatile pchar file = Debug::file[0];
    static volatile int line = Debug::line[0];
    
    (void)file;
    (void)line;

    while (1) //-V776
    {
        file = Debug::file[0];
        line = Debug::line[0];
        
        file = Debug::file[0]; //-V570 //-V1048
        line = Debug::line[0]; //-V570 //-V1048
    }
}

__attribute((noreturn)) void MemManage_Handler(void)
{
    while (1)
    {
    }
}

__attribute((noreturn)) void BusFault_Handler(void)
{
    while (1)
    {
    }
}

__attribute((noreturn)) void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{

}

void DebugMon_Handler(void)
{

}

void PendSV_Handler(void)
{

}

void SysTick_Handler(void)
{
    HAL_IncTick();
}


void OTG_FS_IRQHandler()
{
    HAL_PCD_IRQHandler(&VCP::handlePCD);
}


void OTG_HS_IRQHandler()
{
    HAL_HCD_IRQHandler((HCD_HandleTypeDef *)FDrive::handleHCD);
    
    FDrive::OnHandler_OTG_HS();
}


void DMA1_Stream5_IRQHandler()
{
    HAL_DMA_IRQHandler(((DAC_HandleTypeDef *)Beeper::handle)->DMA_Handle1);
}

#ifdef __cplusplus
}
#endif
