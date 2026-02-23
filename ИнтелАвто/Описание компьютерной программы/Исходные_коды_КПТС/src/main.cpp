// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/HAL/HAL.h"
#include "Menu/Menu.h"
#include "Hardware/Timer.h"
#include "Display/Display_.h"
#include "Hardware/Keyboard/Keyboard_.h"
#include "FlashDrive/FlashDrive.h"
#include "LAN/LAN.h"
#include "Device/IT6523.h"
#include "VCP/VCP.h"
#include "Hardware/Beeper.h"
#include "SCPI/SCPI.h"
#include "Device/Device.h"


#ifndef WIN32
    #if __ARMCC_VERSION != 6210000
        // На других версиях компиляторов не проверялось
        // Но на 6.23 из Keil 5.42a не запускается из-за new, malloc
        #error "Requires ARM Compiler V6.21 from uVision 5.39"
    #endif
#endif


// На эту строку stm32f4xx_hal_eth.c : 1148 должно заходить при подключении
// HAL_ETH_RxLinkCallback(&heth->RxDescList.pRxStart, &heth->RxDescList.pRxEnd,



int main(void)
{
    HAL::Init();

    gset.Load();

    LAN::Init();
    
    Display::Init();

    Keyboard::Init();

    Menu::Init();

    VCP::Init();

    FDrive::Init();

    Beeper::Init();
    
    while (1)
    {
        DEBUG_POINT_0

        Device::TasksUpdate();
        DEBUG_POINT_0
        Menu::Input::Update();
        DEBUG_POINT_0

        Device::TasksUpdate();
        DEBUG_POINT_0;
        Display::Update();
        DEBUG_POINT_0;
        Device::TasksUpdate();
        DEBUG_POINT_0;
        Timer::UpdateTasks();
        DEBUG_POINT_0;

        Device::TasksUpdate();
        DEBUG_POINT_0;
        IT6523::Update();
        DEBUG_POINT_0;
        Device::TasksUpdate();
        DEBUG_POINT_0;
        FDrive::Update();
        DEBUG_POINT_0;
        Device::TasksUpdate();
        DEBUG_POINT_0;
        SCPI::Update();
        DEBUG_POINT_0;
        gset.Save();
        DEBUG_POINT_0;
    }
}
