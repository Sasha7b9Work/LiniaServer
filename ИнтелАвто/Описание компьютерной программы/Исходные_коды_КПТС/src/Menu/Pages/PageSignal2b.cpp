// 2024/01/04 11:55:06 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/Pages/Pages.h"
#include "Settings/Settings.h"
#include "Menu/Menu.h"
#include "Menu/MenuItemsDef.h"
#include "Display/Text_.h"
#include "Display/Display_.h"
#include "LAN/ServerTCP.h"
#include "Device/IT6523.h"
#include "Device/Device.h"


namespace PageSignal2b
{
    TParameter param_td("td",
        &gset.signals[TypePicture::_2b_SAEJ1113].values12[0], Time(200), Time(2000),
        &gset.signals[TypePicture::_2b_SAEJ1113].values24[0], Time(200), Time(2000),
        250, 165);

    static CParameter param_N("N",
        &gset.signals[TypePicture::_2b_SAEJ1113].values12[1], Counter(1), Counter(10),
        &gset.signals[TypePicture::_2b_SAEJ1113].values24[1], Counter(1), Counter(10));

    static void FuncPress_Signal()
    {
        PageMain::SetPage(PageSignal3a::self);
    }

    DEF_BUTTON(bSignal2b,
        "Сигнал",
        FuncPress_Signal
    );

    DEF_CHOICE_2(chModeVoltage,
        "12В", "24В",
        gset.voltage_mode,
        FuncVV
    );

    static bool FuncStartTest()
    {
        IT6523::Start(TypePicture::_2b_SAEJ1113, param_N.GetValue().ToMU());

        return true;
    }

    static Item *items[] =
    {
        &bSignal2b,
        &chModeVoltage,
        &param_td,
        &param_N,
        nullptr
    };

    static void AdditionDraw()
    {
        Text("%s     Ri: 0.01 Ом     Импульсов: %d", VoltageMode::TextValue(),
            Device::IsRunning() ? IT6523::RemainedPulses() : param_N.GetValue().ToMU()).Write(
            Display::xConstParameters, Display::yConstParameters, Color::WHITE
        );

        Page::DrawConnectionServer();
    }

    static Page page(items, AdditionDraw, FuncStartTest);
}


Page *PageSignal2b::self = &PageSignal2b::page;
