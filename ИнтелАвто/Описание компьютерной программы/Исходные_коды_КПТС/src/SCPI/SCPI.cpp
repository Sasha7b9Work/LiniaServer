// 2025/02/11 09:42:07 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "SCPI/SCPI.h"
#include "SCPI/RingBuffer.h"
#include "SCPI/Commands.h"
#include "VCP/VCP.h"
#include "Menu/MenuItems.h"
#include "Menu/Pages/Pages.h"
#include "Utils/StringUtils_.h"
#include "Menu/Menu.h"
#include "Settings/Settings.h"
#include "LAN/ClientTCP.h"
#include <cctype>


namespace SCPI
{
    void Send(DirectionSCPI::E, pchar);

    void Error(DirectionSCPI::E, pchar);
    // Выдаётся, когда параметр не может быть установлен на данном сигнале
    static void ErrorBabSignal();

    static RingBuffer ring_bufferUSB;   // Сюда ложим принятые данные прямо в прерывании
    static RingBuffer ring_bufferLAN;

    static InBuffer in_bufferUSB(DirectionSCPI::USB);       // А сюда перекладываем из ring_buffer в функции SCPI::Update()
    static InBuffer in_bufferLAN(DirectionSCPI::LAN);

    static void SignalSet(DirectionSCPI::E, pchar);
    static void SignalGet(DirectionSCPI::E, pchar);
    static void ModeSet(DirectionSCPI::E, pchar);
    static void ModeGet(DirectionSCPI::E, pchar);
    static void ParamUs(DirectionSCPI::E, pchar);

    struct StructCommand
    {
        pchar message;
        void (*func)(DirectionSCPI::E, pchar);
    };

    static const StructCommand commands[] =
    {
        { ":SIGNAL ",   SignalSet },
        { ":SIGNAL?",   SignalGet },
        { ":MODE ",     ModeSet },
        { ":MODE?",     ModeGet },
        { ":PARAM:US ", ParamUs },
        { nullptr,      nullptr }
    };

    struct StructPageSignal
    {
        pchar name;
        Page *page;
    };

    static const StructPageSignal pages[] =
    {
        { "1",  PageSignal1::self },
        { "2A", PageSignal2a::self },
        { "2B", PageSignal2b::self },
        { "3A", PageSignal3a::self },
        { "3B", PageSignal3b::self },
        { "4",  PageSignal4::self },
        { "5A", PageSignal5a::self },
        { "5B", PageSignal5b::self },
        { "",   nullptr }
    };

    Page *ExtractPageSignal(pchar);

    static pchar ExtractNamePage(Page *);
}


void SCPI::AppendNewData(DirectionSCPI::E dir, uint8 *buffer, uint size)
{
    if (dir == DirectionSCPI::USB)
    {
        for (uint i = 0; i < size; i++)
        {
            char symbol = (char)*buffer;
            buffer++;

            ring_bufferUSB.Append((uint8)std::toupper(symbol));
        }
    }

    if (dir == DirectionSCPI::LAN)
    {
        for (uint i = 0; i < size; i++)
        {
            char symbol = (char)*buffer;
            buffer++;

            ring_bufferLAN.Append((uint8)std::toupper(symbol));
        }
    }
}


void SCPI::Update()
{
    ring_bufferUSB.GetData(in_bufferUSB);
    in_bufferUSB.Update();

    ring_bufferLAN.GetData(in_bufferLAN);
    in_bufferLAN.Update();
}


void SCPI::InBuffer::Update()
{
    bool run = true;

    while (run)
    {
        Command *command = ExtractCommand();

        run = command->Execute(dir);

        delete command;
    }
}


SCPI::Command *SCPI::InBuffer::ExtractCommand()
{
    while (Size() && (buffer[0] == 0x0d || buffer[0] == 0x0a))
    {
        RemoveFirst(1);
    }

    for (int i = 0; i < Size(); i++)
    {
        if (buffer[i] == 0x0d || buffer[i] == 0x0a)
        {
            if (i == 0)
            {
                break;
            }
            else
            {
                char symbols[64];

                std::memcpy(symbols, buffer, (uint)i);
                symbols[i] = '\0';

                RemoveFirst(i);

                return ParseCommand(symbols);
            }
        }
    }

    return new CommandNull();
}


SCPI::Command *SCPI::InBuffer::ParseCommand(pchar symbols)
{
    char *data = (char *)symbols;

    if (std::strcmp(data, "*IDN?") == 0)                // *IDN?
    {
        return new CommandIDN();
    }

    const StructCommand *command = &commands[0];

    while (command->message)
    {
        uint num_symbols = std::strlen(command->message);

        if (std::memcmp((void *)symbols, (void *)command->message, num_symbols) == 0)
        {
            command->func(dir, symbols + num_symbols);
            return new CommandNull();
        }

        command++;
    }

    SCPI::Error(dir, symbols);

    return new CommandNull();
}


void SCPI::Send(DirectionSCPI::E dir, pchar message)
{
    if (dir == DirectionSCPI::USB)
    {
        VCP::SendStringAsynch0D0A((char *)message);
    }

    if (dir == DirectionSCPI::LAN)
    {
        ClientTCP::SendString((char *)message);
    }
}


void SCPI::Error(DirectionSCPI::E dir, pchar text)
{
    if (dir == DirectionSCPI::USB)
    {
        VCP::SendStringAsynchRAW((char *)"ERROR !!! Unknown sequence : ");
        VCP::SendStringAsynch0D0A((char *)text);
    }

    if (dir == DirectionSCPI::LAN)
    {
        ClientTCP::SendStringRAW((char *)"ERROR !!! Unknown sequence : ");
        ClientTCP::SendString((char *)text);
    }
}


void SCPI::ErrorBabSignal()
{
    VCP::SendStringAsynch0D0A("ERROR !!! Not exist parameter for signal %s", ExtractNamePage(Menu::OpenedPage()));
}


void SCPI::SignalSet(DirectionSCPI::E dir, pchar params)
{
    const StructPageSignal *chan = &pages[0];

    while (chan->page)
    {
        if (SU::EqualsZeroStrings(chan->name, params))
        {
            PageMain::SetPage(chan->page);
            return;
        }

        chan++;
    }

    SCPI::Error(dir, params);
}


void SCPI::SignalGet(DirectionSCPI::E dir, pchar)
{
    const StructPageSignal *chan = &pages[0];

    while (chan->page)
    {
        if (chan->page == Menu::OpenedPage())
        {
            SCPI::Send(dir, chan->name);
            return;
        }

        chan++;
    }
}


void SCPI::ModeSet(DirectionSCPI::E dir, pchar params)
{
    if (SU::EqualsZeroStrings(params, "12V"))
    {
        gset.voltage_mode.current = VoltageMode::_12;
    }
    else if (SU::EqualsZeroStrings(params, "24V"))
    {
        gset.voltage_mode.current = VoltageMode::_24;
    }
    else
    {
        Error(dir, params);
    }
}


void SCPI::ModeGet(DirectionSCPI::E dir, pchar params)
{
    if (params[0])
    {
        Error(dir, params);
    }
    else
    {
        SCPI::Send(dir, VoltageMode::Is12() ? "12V" : "24V");
    }
}


Page *SCPI::ExtractPageSignal(pchar params)
{
    const StructPageSignal *page = &pages[0];

    while (page->page)
    {
        if (std::strcmp(params, page->name) == 0)
        {
            return page->page;
        }
        page++;
    }

    return nullptr;
}


pchar SCPI::ExtractNamePage(Page *p)
{
    const StructPageSignal *page = &pages[0];

    while (page->page)
    {
        if (page->page == p)
        {
            return page->name;
        }
        page++;
    }

    return "";
}


void SCPI::ParamUs(DirectionSCPI::E dir, pchar params)
{
    float value = 0.0f;

    if (!SU::StringToFloat(params, &value))
    {
        Error(dir, params);
    }
    else
    {
        VParameter *Us = VParameter::CurrentUs();

        if (Us == nullptr)
        {
            ErrorBabSignal();
        }
        else
        {

        }
    }
}
