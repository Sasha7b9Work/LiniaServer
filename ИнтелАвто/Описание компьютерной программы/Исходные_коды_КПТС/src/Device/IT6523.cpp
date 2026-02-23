// 2025/02/27 10:37:22 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Device/IT6523.h"
#include "LAN/ServerTCP.h"
#include "Hardware/Timer.h"
#include "Hardware/Keyboard/Keyboard_.h"
#include "Device/Device.h"
#include "Menu/Pages/Pages.h"
#include "Utils/StringUtils_.h"
#include "LAN/LAN.h"
#include <cstdarg>
#include <cstdio>


namespace IT6523
{
    static bool is_connected = false;
    static uint time_connect = 0;
    static int pulses_remained = 0;                 // Осталось импульсов
    static bool in_pause = false;
    static uint period_heavy_impulses = 10 * 1000;  // Время между "тяжёлыми" импульсами - теми, которые выделяют много тепла (5а, 5б)
    static uint time_last_heavy_impulse = 0;        // Время запуска последнего "тяжёлого" импульса

    static TypePicture::E actual = TypePicture::_Count;

    int RemainedPulses()
    {
        return pulses_remained;
    }

    void SendSequence(int num_step, int volts, int ms);
}


void IT6523::Update()
{
    // \todo Эти пляски нужны потому, что сразу после подключения к серверу он не получает команду
    {
        if (!ServerTCP::IsConnected())
        {
            is_connected = false;

            time_connect = 0;

            return;
        }

        if (time_connect == 0)
        {
            time_connect = TIME_MS;
        }

        if (!is_connected)
        {
            if (TIME_MS - time_connect < 5000)
            {
                return;
            }

            is_connected = true;

            SendCommand("SYSTem:REMote");
            SendCommand("trigger:source bus");
        }
    }
}


void IT6523::SendCommand(pchar message)
{
    static uint time_next = 0;
    
    while(TIME_MS < time_next)
    {
        Device::TasksUpdate();
    }
    
    time_next = TIME_MS;
    
    if (message[0] != '*')
    {
        ServerTCP::SendString(":");
    }

    ServerTCP::SendString(message);

    ServerTCP::SendString("\x0d\x0a");
}


void IT6523::SendCommandF(pchar format, ...)
{
    static char buffer[128];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    SendCommand(buffer);
}


bool IT6523::Start(const Value &current)
{
    if (!TypePicture::IsExtern())
    {
        if (gset.voltage_mode.Is12())
        {
            SendCommandF("VOLT 13.5");
        }
        else
        {
            SendCommandF("VOLT 27");
        }

        char str_A[32];
        SU::MilliUnitsToUnits(current.ToMU(), str_A);

        SendCommandF("CURR %s", str_A);
    }
    else
    {
        return false;
    }

    SendCommand("SOURCE:OUTPut:STATE 1");

    return true;
}

bool IT6523::Start(TypePicture::E type, int num_pulses)
{
    actual = type;

    pulses_remained = num_pulses;

    uint period = 5000;

    char buffer[32];

    if (actual == TypePicture::_2b_SAEJ1113)
    {
        IT6523::SendCommandF("RES 0.01");
        IT6523::SendCommandF("carwave:sae:2b:volt %dV", gset.voltage_mode.CurrentVolts());
        char str_Uakb[32];
        IT6523::SendCommandF("carwave:sae:2b:TD %s", SU::MilliUnitsToUnits(PageSignal2b::param_td.GetValue().ToMU(), str_Uakb));
        IT6523::SendCommand("carwave:sae:2b:state 1");
    }
    else if (actual == TypePicture::_4_DIN40839)
    {
        IT6523::SendCommandF("RES 0.01");
        IT6523::SendCommand("list:state 0");
        IT6523::SendCommand("sequence:edit 1");
        IT6523::SendCommand("sequence:recall 1");

        const int duration_t9 = PageSignal4::param_t9.GetValue().ToMU();

        period += (uint)duration_t9;

        if (duration_t9 <= 10000)       // Не позволяет генератор в одной команде использовать длительность более 10 сек
        {
            IT6523::SendCommand("sequence:step:count 3");

            char str_Us[32];
            SU::MilliUnitsToUnits(gset.voltage_mode.CurrentVolts() * 1000 - PageSignal4::param_Us.GetValue().ToMU(), str_Us);
            int duration_t7 = PageSignal4::param_t7.GetValue().ToMU();
            IT6523::SendCommandF("seq:volt 1,%s; curr 1,60; width 1,%d ms; slop 1,10 ms", str_Us, duration_t7);

            char str_Ua[32];
            SU::MilliUnitsToUnits(gset.voltage_mode.CurrentVolts() * 1000 - PageSignal4::param_Ua.GetValue().ToMU(), str_Ua);
            IT6523::SendCommandF("seq:volt 2,%s; curr 2,60; width 2,%d ms; slop 2,10 ms", str_Ua, duration_t9);

            char str_Uakb[32];
            SU::MilliUnitsToUnits(gset.voltage_mode.CurrentVolts() * 1000, str_Uakb);
            IT6523::SendCommandF("seq:volt 3,%s; curr 3,60; width 3,10 ms; slop 3,10 ms", str_Uakb);
        }
        else
        {
            IT6523::SendCommand("sequence:step:count 4");

            char str_Us[32];
            SU::MilliUnitsToUnits(gset.voltage_mode.CurrentVolts() * 1000 - PageSignal4::param_Us.GetValue().ToMU(), str_Us);
            int duration_t7 = PageSignal4::param_t7.GetValue().ToMU();
            IT6523::SendCommandF("seq:volt 1,%s; curr 1,60; width 1,%d ms; slop 1,10 ms", str_Us, duration_t7);

            char str_Ua[32];
            SU::MilliUnitsToUnits(gset.voltage_mode.CurrentVolts() * 1000 - PageSignal4::param_Ua.GetValue().ToMU(), str_Ua);
            IT6523::SendCommandF("seq:volt 2,%s; curr 2,60; width 2,%d ms; slop 2,10 ms", str_Ua, 10000);

            IT6523::SendCommandF("seq:volt 3,%s; curr 3,60; width 3,%d ms; slop 3,10 ms", str_Ua, duration_t9 - 10000);

            char str_Uakb[32];
            SU::MilliUnitsToUnits(gset.voltage_mode.CurrentVolts() * 1000, str_Uakb);
            IT6523::SendCommandF("seq:volt 4,%s; curr 4,60; width 4,10 ms; slop 4,10 ms", str_Uakb);
        }

        IT6523::SendCommand("sequence:save 1");

        IT6523::SendCommand("list:edit 1");
        IT6523::SendCommand("list:repeat 1");
        IT6523::SendCommand("list:sequence:count 1");
        IT6523::SendCommand("list:sequence:select 1,1");
        IT6523::SendCommand("list:save 1");

        IT6523::SendCommand("list:recall 1");
        IT6523::SendCommand("list:state 1");
    }
    else if (actual == TypePicture::_5a_16750_1)
    {
        if (time_last_heavy_impulse != 0 && TIME_MS < time_last_heavy_impulse + period_heavy_impulses)
        {
            return false;
        }

        period = period_heavy_impulses;
        IT6523::SendCommandF("RES %d.0", PageSignal5a::param_Ri.GetValue().ToMU());
        IT6523::SendCommand("carwave:iso16750:load:dump:test amode");
        IT6523::SendCommandF("carwave:iso16750:load:dump:volt %dV", gset.voltage_mode.CurrentVolts());
        IT6523::SendCommandF("carwave:iso16750:load:dump:UN %s", SU::MilliUnitsToUnits(PageSignal5a::param_Us.GetValue().ToMU(), buffer));
        IT6523::SendCommandF("carwave:iso16750:load:dump:TD %s", SU::MilliUnitsToUnits(PageSignal5a::param_td.GetValue().ToMU(), buffer));
        IT6523::SendCommandF("carwave:iso16750:load:dump:state 1");
        time_last_heavy_impulse = TIME_MS;
    }
    else if (actual == TypePicture::_5b_16750_2)
    {
        if (time_last_heavy_impulse != 0 && TIME_MS < time_last_heavy_impulse + period_heavy_impulses)
        {
            return false;
        }

        period = period_heavy_impulses;
        IT6523::SendCommandF("RES %d.0", PageSignal5b::param_Ri.GetValue().ToMU());
        IT6523::SendCommand("carwave:iso16750:load:dump:test bmode");
        IT6523::SendCommandF("carwave:iso16750:load:dump:volt %dV", gset.voltage_mode.CurrentVolts());
        IT6523::SendCommandF("carwave:iso16750:load:dump:UN %s", SU::MilliUnitsToUnits(PageSignal5b::param_Us.GetValue().ToMU(), buffer));
        IT6523::SendCommandF("carwave:iso16750:load:dump:US %d", gset.voltage_mode.Is12() ? 35 : 59);
        IT6523::SendCommandF("carwave:iso16750:load:dump:TD %s", SU::MilliUnitsToUnits(PageSignal5b::param_td.GetValue().ToMU(), buffer));
        IT6523::SendCommandF("carwave:iso16750:load:dump:state 1");
        time_last_heavy_impulse = TIME_MS;
    }

    SendCommand("SOURCE:OUTPut:STATE 1");

    Timer::SetPeriodicTask(TimerTask::IT6523, period, IT6523::CallbackOnTimerImpulse);

    return true;
}


void IT6523::CallbackOnTimerImpulse()
{
    if (!in_pause)
    {
        SendCommand("*trg");

        if (--pulses_remained == 0)
        {
            Device::Stop();
        }
    }
}


void IT6523::Pause()
{
    in_pause = true;
}


void IT6523::Resume()
{
    in_pause = false;
}


void IT6523::Stop()
{
    Timer::DisableTask(TimerTask::IT6523);

    if (actual == TypePicture::_2b_SAEJ1113)
    {
        IT6523::SendCommand("carwave:sae:2b:state 0");
    }
    else if (actual == TypePicture::_4_DIN40839)
    {
        IT6523::SendCommand("carwave:startup:din40839:state 0");
    }
    else if (actual == TypePicture::_5a_16750_1 ||
        actual == TypePicture::_5b_16750_2)
    {
        IT6523::SendCommandF("carwave:iso16750:load:dump:state 0");
    }

    SendCommand("SOURCE:OUTPut:STATE 0");

    Keyboard::AddKey(Key::Stop);
}


uint IT6523::TimeLeftToHeavyImpulse()
{
    if (TIME_MS >= time_last_heavy_impulse + period_heavy_impulses)
    {
        return 0;
    }

    return (period_heavy_impulses - (TIME_MS - time_last_heavy_impulse)) / 1000;
}
