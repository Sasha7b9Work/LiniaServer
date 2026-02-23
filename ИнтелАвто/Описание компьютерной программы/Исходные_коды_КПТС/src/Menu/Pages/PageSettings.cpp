// 2025/02/03 10:37:50 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/MenuItemsDef.h"
#include "Menu/Pages/Pages.h"
#include "Menu/Menu.h"


namespace PageSettings
{
    static void FuncPress_Back()
    {
        Menu::SetOpenedPage(PageMain::self);
    }

    DEF_BUTTON(bBack,
        STR_BACK,
        FuncPress_Back
    );

    static void FuncPress_LAN()
    {
        Menu::SetOpenedPage(PageLAN::self);
    }

    DEF_BUTTON(bLAN,
        "LAN",
        FuncPress_LAN
    );

    DEF_CHOICE_2_FULL(chTypePicture,
        "Изображение",
        "Сигнал", "Схема",
        gset.type_picture,
        FuncVV
    );

    uint8 show_debug_voltage = 0;

    DEF_CHOICE_2_FULL(chDebugU,
        "Напряжение",
        "Нет", "Да",
        show_debug_voltage,
        FuncVV
    );

    static Item *items[] =
    {
        &bBack,
        &bLAN,
        &chTypePicture,
//        &chDebugU,
        nullptr
    };

    static Page page(items, nullptr, nullptr);

    Page *self = &page;
}
