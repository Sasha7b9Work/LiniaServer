// 2025/02/21 08:42:10 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/MenuItemsDef.h"
#include "Menu/Pages/Pages.h"
#include "Menu/Menu.h"


namespace PageLAN
{
    DEF_BUTTON(bBack,
        STR_BACK,
        []()
        {
            Menu::SetOpenedPage(PageSettings::self);
        }
    );

    DEF_BUTTON(bIT6523,
        "IT6523",
        []()
        {
            Menu::SetOpenedPage(PageIT6523::self);
        }
    );

    DEF_BUTTON(bSCPI,
        "SCPI",
        []()
        {
            Menu::SetOpenedPage(PageSCPI::self);
        }
    );

    FieldIP bNetmask{
        "Маска сети",
        gset.netmask
    };

    FieldIP bGateway{
        "Gateway",
        gset.gw
    };

    FieldMAC bMAC{
        "MAC",
        gset.mac
    };

    static Item *items[] =
    {
        &bBack,
        &bIT6523,
        &bSCPI,
        &bNetmask,
        &bGateway,
        &bMAC,
        nullptr
    };

    static Page page(items, nullptr, nullptr);

    Page *self = &page;


    namespace PageIT6523
    {
        DEF_BUTTON(bBack,
            STR_BACK,
            []()
            {
                Menu::SetOpenedPage(PageLAN::self);
            }
        );

        FieldIP bIP{
            "Адрес IP",
            gset.ipIT6523
        };

        FieldPort bPort{
            "Порт",
            &gset.portIT6523
        };

        static Item *items[] =
        {
            &bBack,
            &bIP,
            &bPort,
            nullptr
        };

        static Page page(items, nullptr, nullptr);

        Page *self = &page;
    }


    namespace PageSCPI
    {
        DEF_BUTTON(bBack,
            STR_BACK,
            []()
            {
                Menu::SetOpenedPage(PageLAN::self);
            }
        );

        FieldIP bIP{
            "Адрес IP",
            gset.ipSCPI
        };

        FieldPort bPort{
            "Порт",
            &gset.portSCPI
        };

        static Item *items[] =
        {
            &bBack,
            &bIP,
            &bPort,
            nullptr
        };

        static Page page(items, nullptr, nullptr);

        Page *self = &page;
    }
}
