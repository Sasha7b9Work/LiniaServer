// 2025/02/11 13:07:30 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/Pages/Pages.h"
#include "Menu/MenuItemsDef.h"
#include "Menu/Menu.h"
#include "Display/Text_.h"


namespace PageInfo
{
    static void FuncPress_Back()
    {
        Menu::SetOpenedPage(PageMain::self);
    }

    DEF_BUTTON(bBack,
        STR_BACK,
        FuncPress_Back
    );

    static Item *items[] =
    {
        &bBack,
        nullptr
    };

    static void Func_Draw()
    {
        int x = 170;
        int y = 60;
        int dy = 50;

        Text("Генератор импульсов КПТС").Write(x, y, Color::WHITE);

        Text("Версия : %d", VERSION_BUILD).Write(x, y + dy);

        Text("Дата : %s", DATE_BUILD).Write(x, y + dy * 2);
    }

    static Page pages(items, Func_Draw, nullptr);

    Page *self = &pages;
}
