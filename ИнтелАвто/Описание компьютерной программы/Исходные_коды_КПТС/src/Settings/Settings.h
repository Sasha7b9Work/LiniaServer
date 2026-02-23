// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Connector/Device/Value_.h"
#include "Display/Colors_.h"


class Parameter;


// Используется для расчёта длительности теста
struct Duration
{
    Duration(const Parameter &period, const Parameter &N);
    Duration(float periodSec, const Parameter &N);

    pchar ToStringValue(char buffer[128]) const;

private:
    uint timeMS;
};


struct VoltageMode
{
    enum E
    {
        _12,
        _24,
        Count
    };

    static E Current();

    static bool Is12() { return Current() == _12; }

    int CurrentVolts()
    {
        return Is12() ? 12 : 24;
    }

    static pchar TextValue();

    E current;
};


struct TypePicture
{
    enum E
    {
        _1,             // Int
        _2a,            // Int
        _2b_SAEJ1113,   // Ext 
        _3a,            // Int
        _3b,            // Int
        _4_DIN40839,    // Ext Программно формируемый сигнал командами LIST
        _5a_16750_1,    // Ext
        _5b_16750_2,    // Ext
        Scheme1,
        Scheme2,
        _Count
    };

    static pchar Name();

    static void Set(E v) { current = v; }

    static bool Is(E v) { return current == v; }

    static pchar ToString();

    static E Current() { return current; }

    // true, если сигнал формируется внешним источником
    static bool IsExtern();

private:

    static E current;
};


// Стиль ГУИ - как отображать симолы
struct StyleGUI
{
    enum E
    {
        Modern,     // Обычные символы
        Classic     // Сегментные символы
    } value;

    StyleGUI(E v) : value(v) {}

    bool IsModern() const { return value == Modern; }
    void Switch() { value = (value == Modern) ? Classic : Modern; }
};


struct SettingsSignal
{
    static const int MAX_PARAMS = 5;

    Value values12[MAX_PARAMS];
    Value values24[MAX_PARAMS];
};


struct SettingsCal
{
    struct StructCal
    {
        float offset;
        float k;

        void Reset();
    };

    // 8 типов сигналов
    // 2 - 12/24В
    // 4 - четыре отрезка для калибровки. Пока используем один
    StructCal cal[TypePicture::_Count][VoltageMode::Count][4];
};


struct Settings
{
    uint           size;
    ColorScheme    schemes[ColorScheme::COUNT];
    StyleGUI       styleGUI;
    uint8          colorScheme;
    TypePicture::E signal;
    VoltageMode    voltage_mode;
    SettingsSignal signals[TypePicture::_Count];
    uint16         portSCPI;                            // По этому порту подключается внешний клиент для команд SCPI
    uint16         portIT6523;                          // По этому порту подключаемся к IT6523
    uint8          ipIT6523[4];
    uint8          ipSCPI[4];
    uint8          mac[6];
    uint8          gw[4];
    uint8          netmask[4];
    SettingsCal    cal;
    uint8          type_picture;                        // Что показывать на экране. 0 - сигнал, 1 - схема

    void Save();
    void Load();
    void Reset();
};

extern Settings gset;
