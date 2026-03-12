// 2023/12/25 17:20:32 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/MenuItemsDef.h"
#include "Menu/Pages/Pages.h"
#include "Settings/Settings.h"
#include "Menu/Menu.h"
#include "Connector/Device/Messages_.h"
#include "Display/Text_.h"
#include "Display/Display_.h"
#include "Hardware/Timer.h"
#include "Device/IT6523.h"


namespace PageSignal1
{
    // Амплитуда
    VParameter param_Us("Us",
        &gset.signals[TypePicture::_1].values12[0], Voltage(us12_min * 1000), Voltage(us12_max * 1000),
        &gset.signals[TypePicture::_1].values24[0], Voltage(us24_min * 1000), Voltage(us24_max * 1000),
        280, 155);

    static AParameter param_A("Imax",
        &gset.signals[TypePicture::_1].values12[3], Current(Current::min * 1000), Current(Current::max * 1000),
        &gset.signals[TypePicture::_1].values24[3], Current(Current::min * 1000), Current(Current::max * 1000));

    // Период повторения
    static TParameter param_t1("Период",
        &gset.signals[TypePicture::_1].values12[1], Time(500), Time(5000),
        &gset.signals[TypePicture::_1].values24[1], Time(500), Time(5000),
        230, 60);

    // Количество импульсов
    static CParameter param_N("N",
        &gset.signals[TypePicture::_1].values12[2], Counter(5000), Counter(100000),
        &gset.signals[TypePicture::_1].values24[2], Counter(5000), Counter(100000));

    static void FuncPress_Signal()
    {
        PageMain::SetPage(PageSignal2a::self);
    }

    DEF_BUTTON(bSignal1,
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
        if (VoltageMode::Is12())
        {
            Message::Start1_12V(param_Us.GetCalibrateValue(TypePicture::_1, VoltageMode::_12), param_t1.GetValue()).Transmit();
        }
        else
        {
            Message::Start1_24V(param_Us.GetCalibrateValue(TypePicture::_1, VoltageMode::_24), param_t1.GetValue()).Transmit();
        }

        IT6523::Start(param_A.GetValue());

        RemainingTimeCounter::Start(param_t1.GetValue().ToMU(), param_N);

        return true;
    }


    static Item *items[] =
    {
        &bSignal1,
        &chModeVoltage,
        &param_Us,
        &param_t1,
        &param_N,
        &param_A,
        nullptr
    };

    static void AdditionDraw()
    {
        char buffer[128];

        Text("%s     Ri: %s     Длит: %s", VoltageMode::TextValue(), VoltageMode::Is12() ? "10 Ом" : "50 Ом", Duration(param_t1, param_N).ToStringValue(buffer)).Write(
            Display::xConstParameters, Display::yConstParameters, Color::WHITE
        );

        Page::DrawConnectionServer();
    }

    static Page page(items, AdditionDraw, FuncStartTest);
}


Page *PageSignal1::self = &PageSignal1::page;
