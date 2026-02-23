// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Display/Primitives_.h"
#include "Hardware/Keyboard/Keyboard_.h"


class Page;


namespace Menu
{
    void Init();

    void Draw();

    Page* OpenedPage();

    void SetOpenedPage(Page *page);

    // Если сейчас открыта страница сигнала, то true
    bool OpenedPageIsSignal();

    namespace Input
    {
        // Эта функция вызывается главном цикле
        void Update();

        // Установка функции обработки ввода
        void SetFuncUpdate(void (*funcUpdate)());

        // Это функция обработки обработки по умолчанию
        void FuncUpdate();

        void FuncEmptyUpdate();

        // Обработка события кнопки
        void OnKey(Key::E);
    };

    extern Primitives::Label labelMode;
};
