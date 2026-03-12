// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/Timer.h"
#include "Hardware/HAL/HAL.h"
#include "Connector/Device/Transceiver_.h"
#include "Connector/Device/Messages_.h"
#include "Display/Display_.h"


namespace Transceiver
{
    void Delay();
}


void Transceiver::Transmit(BaseMessage *message)
{
    (void)message;

#ifndef GUI

    uint recv_crc = (uint)-1;

    int counter = 0;

    while (recv_crc != message->CalculateCRC())
    {
        counter++;

        HAL_SPI1::CS(true);

        HAL_SPI1::TransmitUInt((uint)message->Size());              // Передаём размер сообщения (4 байта)

        Delay();

        HAL_SPI1::Transmit(message->TakeData(), message->Size());   // Передаём сообщение

        Delay();

        HAL_SPI1::TransmitUInt(message->CalculateCRC());            // Передаём контрольную сумму сообщения

        Delay();

        recv_crc = HAL_SPI1::ReceiveUInt();

        HAL_SPI1::CS(false);

        Display::num_sends = counter;

        if ((counter % 10) == 0)
        {
            Display::Update();
        }
    }

    Display::num_sends = counter;

#endif
}


void Transceiver::Delay()
{
    HAL_TIM::DelayMS(2);
}
