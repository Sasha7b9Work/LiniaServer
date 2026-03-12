// 2024/01/04 14:11:59 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/Pages/Pages.h"
#include "Menu/MenuItemsDef.h"
#include "Settings/Settings.h"
#include "Menu/Menu.h"
#include "Display/Text_.h"
#include "Display/Display_.h"
#include "Device/IT6523.h"
#include "Device/Device.h"


namespace PageSignal4
{
    static CParameter param_N("N",
        &gset.signals[TypePicture::_4_DIN40839].values12[0], Counter(1), Counter(10),
        &gset.signals[TypePicture::_4_DIN40839].values24[0], Counter(1), Counter(10));

    VParameter param_Us("Us",
        &gset.signals[TypePicture::_4_DIN40839].values12[1], Voltage(6000), Voltage(7500),
        &gset.signals[TypePicture::_4_DIN40839].values24[1], Voltage(12000), Voltage(16000),
        200, 110);

    VParameter param_Ua("Ua",
        &gset.signals[TypePicture::_4_DIN40839].values12[2], Voltage(2500), Voltage(6000),
        &gset.signals[TypePicture::_4_DIN40839].values24[2], Voltage(5000), Voltage(12000),
        360, 90);

    TParameter param_t7("t7",
        &gset.signals[TypePicture::_4_DIN40839].values12[3], Time(15), Time(40),
        &gset.signals[TypePicture::_4_DIN40839].values24[3], Time(50), Time(100),
        230, 150);

    TParameter param_t9("t9",
        &gset.signals[TypePicture::_4_DIN40839].values12[4], Time(500), Time(20000),
        &gset.signals[TypePicture::_4_DIN40839].values24[4], Time(500), Time(20000),
        345, 135);

    static void FuncPress_Signal()
    {
        PageMain::SetPage(PageSignal5a::self);
    }

    DEF_BUTTON(bSignal5a,
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
        IT6523::Start(TypePicture::_4_DIN40839, param_N.GetValue().ToMU());

        return true;
    }

    static Item *items[] =
    {
        &bSignal5a,
        &chModeVoltage,
        &param_N,
        &param_Us,
        &param_Ua,
        &param_t7,
        &param_t9,
        nullptr
    };

    static void AdditionDraw()
    {
        Text("%s    Ri: 0.01 Ом    Импульсов: %d",
            VoltageMode::TextValue(),
            Device::IsRunning() ? IT6523::RemainedPulses() : param_N.GetValue().ToMU()
        ).Write(
            Display::xConstParameters, Display::yConstParameters, Color::WHITE
        );

        Page::DrawConnectionServer();
    }

    static Page page(items, AdditionDraw, FuncStartTest);
}


Page *PageSignal4::self = &PageSignal4::page;
