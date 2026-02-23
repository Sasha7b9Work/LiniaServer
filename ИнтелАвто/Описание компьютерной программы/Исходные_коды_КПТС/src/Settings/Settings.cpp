// 2023/09/08 16:18:29 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Settings.h"
#include "Hardware/HAL/HAL.h"
#include "Menu/MenuItems.h"
#include "Utils/StringUtils_.h"
#include "Hardware/Timer.h"
#include <cstdio>
#include <cstring>


TypePicture::E TypePicture::current = TypePicture::_1;

static ColorScheme def_colors[3] =
{
    {
        Color::Make(248, 248, 248),     // FILL             0
        Color::Make(0,   24,  0),       // BACK             1
        Color::Make(24,  24,  24),      // BACK_UNSELECT    2
        Color::Make(192, 192, 192),     // BACK_SELECT      3
        Color::Make(248, 0,   0),       // RED              4
        Color::Make(0,   248, 0),       // GREEN            5
        Color::Make(0,   0,   248),     // BLUE             6
        Color::Make(248, 248, 248),     // WHITE            7
        Color::Make(0,   0,   0),       // EMPTY            8
        Color::Make(127, 127, 127),     // GRAY             9
        Color::Make(248, 248, 0),       // YELLOW          10
        Color::Make(192, 192, 192)      // GRAY_75         11
    },
    {
        Color::Make(248, 248, 248),     // FILL             0
        Color::Make(0,   0,   0),       // BACK             1
        Color::Make(0,   56,  0),       // BACK_UNSELECT    2
        Color::Make(240, 160, 48),      // BACK_SELECT      3
        Color::Make(248, 0,   0),       // RED              4
        Color::Make(0,   248, 0),       // GREEN            5
        Color::Make(0,   0,   248),     // BLUE             6
        Color::Make(248, 248, 248),     // WHITE            7
        Color::Make(112, 8,   8),       // EMPTY            8
        Color::Make(127, 127, 127),     // GRAY             9
        Color::Make(248, 248, 0),       // YELLOW          10
        Color::Make(192, 192, 192)      // GRAY_75         11
    },
    {
        Color::Make(248, 248, 248),     // FILL             0
        Color::Make(0,   0,   88),      // BACK_1           1
        Color::Make(24,  24,  24),      // BACK_UNSELECT    2
        Color::Make(192, 192, 192),     // BACK_SELECT      3
        Color::Make(248, 0,   0),       // RED              4
        Color::Make(0,   248, 0),       // GREEN            5
        Color::Make(0,   0,   248),     // BLUE             6
        Color::Make(248, 248, 248),     // WHITE            7
        Color::Make(0,   0,   0),       // EMPTY            8
        Color::Make(127, 127, 127),     // GRAY             9
        Color::Make(248, 248, 0),       // YELLOW          10
        Color::Make(192, 192, 192)      // GRAY_75         11
    }
};


static Settings def_set =
{
    0,                                  // size
    {
        def_colors[0], def_colors[1], def_colors[2]
    },
    StyleGUI(StyleGUI::Modern),
    0,                                          // colorScheme
    TypePicture::_1,
    { VoltageMode::_12 },
    {
        {   {   Voltage(75000),                 // 1 : Us          12      Signal 1
                Time(1100),                     // 1 : t1          12
                Counter(10000),                 // 1 : N           12
                Current(50000),                 // 1 : Ограничение 12
                ValueNull()
            }
            ,
            {   Voltage(300000),                // 1 : Us          24
                Time(1100),                     // 1 : Us          24
                Counter(10000),                 // 1 : N           24
                Current(50000),                 // 1 : Ограничение 24
                ValueNull() }}
        ,
        {   {   Voltage(55000),                 // 2a : Us          12      Signal 2a
                Time(1000),                     // 2a : t1          12
                Counter(10000),                 // 2a : N           12
                Current(50000),                 // 2a : Ограничение 12
                ValueNull()
            }
            ,
            {   Voltage(55000),                 // 2a : Us          24
                Time(1000),                     // 2a : t1          24
                Counter(10000),                 // 2a : N           24
                Current(50000),                 // 2a : Ограничение 24
                ValueNull() }}
        ,
        {   {   Time(500),                      // 2b : t1  12      Signal 2b
                Counter(10),
                ValueNull(),
                ValueNull(),
                ValueNull()
            }
            ,
            {   Time(500),                      // 2b : t1 24
                Counter(10),
                ValueNull(),
                ValueNull(),
                ValueNull() }}
        ,
        {   {   Voltage(112000),                // 3a : Us          12       Signal 3a
                Counter(10000),                 // 3a : N           12
                Current(50000),                 // 3a : Ограничение 12
                ValueNull(),
                ValueNull()
            }
            ,
            {   Voltage(150000),                // 3a : Us          24
                Counter(10000),                 // 3a : N           24
                Current(50000),                 // 3a : Ограничение 24
                ValueNull(),
                ValueNull() }}
        ,
        {   {   Voltage(75000),                 // 3b : Us          12       Signal 3b
                Counter(10000),                 // 3b : N           12
                Current(50000),                 // 3b : Ограничение 12
                ValueNull(),
                ValueNull()
            }
            ,
            {   Voltage(150000),                // 3b : Us          24
                Counter(10000),                 // 3b : N           24
                Current(50000),                 // 3b : Ограничение 24
                ValueNull(),
                ValueNull() }}
        ,
        {   {   Counter(10),                    // 4 : N
                Voltage(7500),                  // 4 : Us           Signal 4
                Voltage(6000),                  // 4 : Ua
                Time(15),                       // 4 : t7
                Time(2000)}                     // 4 : t9
            ,
            {   Counter(10),                    // 4 : N
                Voltage(16000),                 // 4 : Us
                Voltage(12000),                 // 4 : Ua
                Time(50),                       // 4 : t7
                Time(2000)}}                    // 4 : t9
        ,
        {   {   Voltage(80000),                 // 5a : Us 12       Signal 5a
                Time(54),                       // 5a : td 12
                Counter(10),                    // 5a : N  12
                Counter(10),                    // 5a : Ri 10
                ValueNull()
            }
            ,
            {   Voltage(80000),                 // 5a : Us 24
                Time(100),                      // 5a : td 24
                Counter(10),                    // 5a : N  24
                Counter(10),                    // 5a : Ri 10
                ValueNull() }}
        ,
        { {     Voltage(79000),                 // 5b : Us 12       Signal 5b
                Counter(10),                    // 5b : Ri 10
                Time(54),                       // 5b : td 54
                Counter(5),                     // 5b : N 5
                ValueNull()
            }
            ,
            {   Voltage(101000),                // 5b : Us 24
                Counter(10),                    // 5b : Ri 10
                Time(100),                      // 5b : td 54
                Counter(5),                     // 5b : N 5
                ValueNull() }}
    },
    7,                                          // portSCPI
    30000,                                      // portIT6523
    { 192, 168, 1, 200 },                       // ipIT6523
    { 192, 168, 1, 201 },                       // ipSCPI
    { 0, 1, 2, 3, 4, 5 },                       // mac
    { 192, 168, 1, 1 },                         // gw
    { 255, 255, 255, 0 },                       // netmask
    {
        {
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },
            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            },            {
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}},
                {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}
            }
        }
    },
    1
};


Settings gset = def_set;


void Settings::Load()
{
    HAL_EEPROM::LoadSettings(this);
}


void Settings::Save()
{
    Settings set = def_set;

    if (!HAL_EEPROM::LoadSettings(&set) ||
        std::memcmp(&set, &gset, sizeof(set)) != 0)
    {
        HAL_EEPROM::SaveSettings(this);
    }
}


void Settings::Reset()
{
    gset.schemes[gset.colorScheme] = def_colors[gset.colorScheme];
}


pchar TypePicture::ToString()
{
    static const pchar values[_Count] =
    {
        "1",
        "2a",
        "2b",
        "3a",
        "3b",
        "4",
        "5a",
        "5b",
        "",
        ""
    };

    return values[current];
}


bool TypePicture::IsExtern()
{
    E s = Current();

    return
        s == E::_2b_SAEJ1113 ||
        s == E::_4_DIN40839 ||
        s == E::_5a_16750_1 ||
        s == E::_5b_16750_2;
}


pchar TypePicture::Name()
{
    static const pchar names[_Count] =
    {
        "1",
        "2a",
        "SAEJ1113-11",
        "3a",
        "3b",
        "DIN40839",
        "16750-1",
        "16750-2",
        "",
        ""
    };

    return names[current];
}


VoltageMode::E VoltageMode::Current()
{
    return gset.voltage_mode.current;
}


pchar VoltageMode::TextValue()
{
    static const pchar text[Count] =
    {
        "12В", "24В"
    };

    return text[Current()];
}


Duration::Duration(const Parameter &period, const Parameter &N)
{
    Value val_period = period.GetValue();
    Value val_N = N.GetValue();

    timeMS = (uint)(val_period.ToMU() * val_N.ToMU());
}


Duration::Duration(float periodSec, const Parameter &N)
{
    Value val_N = N.GetValue();

    timeMS = (uint)(periodSec * 1000.f * (float)val_N.ToMU());
}


pchar Duration::ToStringValue(char buffer[128]) const
{
    if (RemainingTimeCounter::InProcess())
    {
        return RemainingTimeCounter::RemainingTime(buffer);
    }

    return SU::TimeMStoText(timeMS, buffer, false);
}


void SettingsCal::StructCal::Reset()
{
    offset = 0.0f;
    k = 1.0f;
}
