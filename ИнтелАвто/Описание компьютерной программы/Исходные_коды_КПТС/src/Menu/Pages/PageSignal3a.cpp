// 2024/01/04 13:56:03 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/Pages/Pages.h"
#include "Menu/Menu.h"
#include "Settings/Settings.h"
#include "Menu/MenuItemsDef.h"
#include "Connector/Device/Messages_.h"
#include "Display/Text_.h"
#include "Display/Display_.h"
#include "Hardware/Timer.h"
#include "Device/IT6523.h"


namespace PageSignal3a
{
    static const int period_ms = 100;

    VParameter param_Us("Us",
        &gset.signals[TypePicture::_3a].values12[0], Voltage(us12_min * 1000), Voltage(us12_max * 1000),
        &gset.signals[TypePicture::_3a].values24[0], Voltage(us24_min * 1000), Voltage(us24_max * 1000),
        275, 125);

    static AParameter param_A("Imax",
        &gset.signals[TypePicture::_3a].values12[2], Current(Current::min * 1000), Current(Current::max * 1000),
        &gset.signals[TypePicture::_3a].values24[2], Current(Current::min * 1000), Current(Current::max * 1000));

    static CParameter param_N("N",
        &gset.signals[TypePicture::_3a].values12[1], Counter(10000), Counter(100000),
        &gset.signals[TypePicture::_3a].values24[1], Counter(10000), Counter(100000));

    static void FuncPress_Signal()
    {
        PageMain::SetPage(PageSignal3b::self);
    }

    DEF_BUTTON(bSignal3a,
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
        Message::Start3A(param_Us.GetCalibrateValue(TypePicture::_3a, VoltageMode::Current())).Transmit();

        RemainingTimeCounter::Start(period_ms, param_N);

        IT6523::Start(param_A.GetValue());

        return true;
    }

    static Item *items[] =
    {
        &bSignal3a,
        &chModeVoltage,
        &param_Us,
        &param_N,
        &param_A,
        nullptr
    };

    static void AdditionDraw()
    {
        char buffer[128];

        Text("%s     Ri: 50 Ом     Длит: %s", VoltageMode::TextValue(), Duration((float)period_ms / 1e3f, param_N).ToStringValue(buffer)).Write(
            Display::xConstParameters, Display::yConstParameters, Color::WHITE
        );

        Page::DrawConnectionServer();
    }

    static Page page(items, AdditionDraw, FuncStartTest);
}


Page *PageSignal3a::self = &PageSignal3a::page;
