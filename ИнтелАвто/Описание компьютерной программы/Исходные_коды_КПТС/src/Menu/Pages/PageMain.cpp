// 2023/12/22 15:42:54 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/MenuItemsDef.h"
#include "Menu/Pages/Pages.h"
#include "Menu/Menu.h"
#include "Display/Display_.h"
#include "FlashDrive/FlashDrive.h"


namespace PageMain
{
    void SetPage(Page *p)
    {
        struct StructPage
        {
            Page          *page;
            TypePicture::E picture;
        };

        static const StructPage pages[] =
        {
            { PageSignal1::self,  TypePicture::_1 },
            { PageSignal2a::self, TypePicture::_2a },
            { PageSignal2b::self, TypePicture::_2b_SAEJ1113 },
            { PageSignal3a::self, TypePicture::_3a },
            { PageSignal3b::self, TypePicture::_3b },
            { PageSignal4::self,  TypePicture::_4_DIN40839 },
            { PageSignal5a::self, TypePicture::_5a_16750_1 },
            { PageSignal5b::self, TypePicture::_5b_16750_2 },
            { nullptr,            TypePicture::_Count }
        };

        const StructPage *page = &pages[0];

        while (page->page)
        {
            if (page->page == p)
            {
                Menu::SetOpenedPage(page->page);
                TypePicture::Set(page->picture);
                break;
            }

            page++;
        }
    }

    static void FuncPress_Tests()
    {
        Menu::SetOpenedPage(PageSignal1::self);
    }

    DEF_BUTTON(bTests,
        "Тесты",
        FuncPress_Tests
    );

    static void FuncPress_Settings()
    {
        Menu::SetOpenedPage(PageSettings::self);
    }

    DEF_BUTTON(bSettings,
        "Настройки",
        FuncPress_Settings
    );

    static void FuncPress_Calibration()
    {
        Menu::SetOpenedPage(PageCalibration::self);
    }

    DEF_BUTTON(bCalibration,
        "Калибровка",
        FuncPress_Calibration
    );

    static void FuncPress_FlashDrive()
    {

    }

    DEF_BUTTON(bFlashDrive,
        "Флешка",
        FuncPress_FlashDrive
    );

    static void FuncPress_Info()
    {
        Menu::SetOpenedPage(PageInfo::self);
    }

    DEF_BUTTON(bInfo,
        "Информация",
        FuncPress_Info
    );

    static void FuncPress_Display()
    {
        Display::Init();
    }


    DEF_BUTTON(bDisplay,
        "Дисплей",
        FuncPress_Display
    );

    static Item *items[] =
    {
        &bTests,
        &bSettings,
//        &bFlashDrive,
        &bCalibration,
        &bInfo,
//        &bDisplay,
        nullptr
    };

    static void OnDraw_Page()
    {
        bFlashDrive.SetActive(FDrive::IsConnected());
    }

    static Page page(items, OnDraw_Page, nullptr);
}


Page *PageMain::self = &PageMain::page;
