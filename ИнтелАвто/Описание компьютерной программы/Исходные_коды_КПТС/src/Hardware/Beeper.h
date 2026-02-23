// 2025/02/11 13:45:16 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Settings/Settings.h"


struct TypeWave
{
    enum E
    {
        Sine,
        Meandr,
        Triangle
    };
};


namespace Beeper
{
    void Init();

    void Stop();

    // Звук нажатия на кнопку.
    void ButtonPress();

    // Функция вызовет звук отпускаемой кнопки только если перед этим проигрывался звук нажатия кнопки.
    void ButtonRelease();

    void GovernorChangedValue();

    void RegulatorShiftRotate();

    void RegulatorSwitchRotate();

    void WarnBeepBad();

    void WarnBeepGood();

    // Эту функцию надо вызывать перед записью/стиранием ППЗУ. Звук конфликтует с ППЗУ.
    void WaitForCompletion();

    extern void *handle;
};
