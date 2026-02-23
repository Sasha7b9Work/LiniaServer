// 2024/02/01 10:16:08 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Device/Device.h"
#include "Menu/MenuItems.h"
#include "Connector/Device/Messages_.h"
#include "Hardware/Timer.h"
#include "Device/IT6523.h"
#include "Display/Colors_.h"
#include "FlashDrive/FlashDrive.h"
#include "LAN/LAN.h"


namespace Device
{
    struct State
    {
        enum E
        {
            Stopped,
            Running,
            Paused
        };
    };

    static State::E state;
}


bool Device::IsStopped()
{
    return state == State::Stopped;
}


bool Device::IsRunning()
{
    return state == State::Running;
}


bool Device::InPause()
{
    return state == State::Paused;
}


bool Device::Start()
{
    if (IsStopped())
    {
        ColorTimer::Reset();

        if (Page::ForCurrentSignal()->StartTest())
        {
            state = State::Running;

            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}


void Device::Pause()
{
    if (IsRunning())
    {
        state = State::Paused;

        Message::Pause().Transmit();

        RemainingTimeCounter::Pause();

        IT6523::Pause();
    }
}


void Device::Resume()
{
    if (InPause())
    {
        state = State::Running;

        Message::Resume().Transmit();

        RemainingTimeCounter::Resume();

        IT6523::Resume();
    }
}


void Device::Stop()
{
    if (Device::IsRunning() || Device::InPause())
    {
        state = State::Stopped;

        Message::Stop().Transmit();

        RemainingTimeCounter::Stop();

        IT6523::Stop();
    }
}


void Device::TasksUpdate()
{
    LAN::Update();
    FDrive::Update();
}
