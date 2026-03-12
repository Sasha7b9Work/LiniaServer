// 2023/09/08 21:53:49 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Settings/Settings.h"
#include "Menu/MenuItems.h"
#include "Menu/Pages/Pages.h"
#include "Display/Primitives_.h"
#include "Display/Display_.h"
#include "Display/Text_.h"
#include "Hardware/Keyboard/Keyboard_.h"
#include "LAN/ServerTCP.h"
#include "Utils/Math_.h"
#include "Menu/Menu.h"
#include "Hardware/HAL/HAL.h"
#include "Utils/StringUtils_.h"
#include "Hardware/Beeper.h"
#include <cstring>
#include <cstdlib>
#include <cmath>


using namespace Primitives;


Parameter *Parameter::editable = nullptr;


int Item::Height()
{
    return 30;
}


int Choice::NumStates() const
{
    int result = 0;

    while (names[result])
    {
        result++;
    }

    return result;
}


void Page::Draw(int x, int y, int, bool)
{
    for (int i = 0; i < NumItems(); i++)
    {
        bool selected = (i == selectedItem);

        Color colorBack = items[i]->ColorBackground(selected);

        Rect(WidthItem(i) - 1, Height() - 1).FillRounded(x, y + 1, 2, colorBack, Color::FILL);

        items[i]->Draw(x, y + 5, WidthItem(i), selected);

        y += Height();
    }

    if (additionalDraw)
    {
        additionalDraw();
    }
}


const Page *Page::ForCurrentSignal()
{
    static const Page *pages[TypePicture::_Count] =
    {
        PageSignal1::self,
        PageSignal2a::self,
        PageSignal2b::self,
        PageSignal3a::self,
        PageSignal3b::self,
        PageSignal4::self,
        PageSignal5a::self,
        PageSignal5b::self,
        nullptr,
        nullptr
    };

    return pages[TypePicture::Current()];
}



bool Page::ConsistOpenedItems() const
{
    for (int i = 0; i < NumItems(); i++)
    {
        if (items[i]->IsOpened())
        {
            return true;
        }
    }

    return false;
}


void Page::DrawParameters() const
{
    for (int i = 0; i < NumItems(); i++)
    {
        if (items[i]->IsParameter())
        {
            Parameter *param = (Parameter *)items[i];

            if (param->IsNowEdited())
            {
                param->Draw(i);

                return;
            }
        }
    }

    for (int i = 0; i < NumItems(); i++)
    {
        if (items[i]->IsParameter())
        {
            Parameter *param = (Parameter *)items[i];

            param->Draw(i);
        }
    }
}


int Page::WidthItem(int num) const
{
    int div = Page::IsSignal(Menu::OpenedPage()) ? 6 : 4;

    int result = ((num % 2) == 0) ? ((Display::PHYSICAL_WIDTH - 1) / div) : (Display::PHYSICAL_WIDTH / div);

    return result;
}


int Page::NumItems() const
{
    int i = 0;
    while (items[i] != nullptr)
    {
        i++;
    }
    return i;
}


void Parameter::Draw(int x0, int y0, int width, bool selected)
{
    ColorDraw(selected).SetAsCurrent();

    if (!selected)
    {
        Color::WHITE.SetAsCurrent();
    }

    Text(Title()).Write(x0, y0 + DeltaTextt(), width);
}


int Parameter::X() const
{
    if (x && (gset.type_picture == 0))
    {
        return x;
    }

    return 90;
}


int Parameter::Y(int num_item) const
{
    if (y && (gset.type_picture == 0))
    {
        return y;
    }

    return Item::Height() * num_item + 10;
}


void Parameter::Draw(int num_item) const
{
    const int width = 82;
    const int height = 21;

    if (IsNowEdited() || IsNowSelected())
    {
        Color color = Color::GetCurrent();

        Rect(width - 2, height - 2).Fill(X() - 2, Y(num_item) - 2, Color::BACK);

        color.SetAsCurrent();
    }

    // Нарисовать редактируемое значение на экране

    if(IsNowEdited())
    {
        ds.Draw(X(), Y(num_item));

        Color color = Color::GetCurrent();

        Color::BACK.SetAsCurrent();

        GetMin().Draw(X(), Y(num_item) + 25);

        GetMax().Draw(X(), Y(num_item) - 25);

        color.SetAsCurrent();
    }
    else
    {
        GetValue().Draw(X(), Y(num_item));
    }

    if (IsNowSelected())
    {
        Color color = Color::GetCurrent();

        Rect(width, height).Draw(X() - 3, Y(num_item) - 3, Color::WHITE);

        color.SetAsCurrent();
    }
}


void Button::Draw(int x, int y, int width, bool selected)
{
    ColorDraw(selected).SetAsCurrent();

    if (!selected)
    {
        Color::WHITE.SetAsCurrent();
    }

    if (!is_active)
    {
        Color::GRAY.SetAsCurrent();
    }

    Text(GetTitle()).Write(x, y + DeltaTextt(), width);
}


void Choice::Draw(int x, int y, int width, bool selected)
{
    ColorDraw(selected).SetAsCurrent();

    if (!selected)
    {
        Color::WHITE.SetAsCurrent();
    }

    if (title)
    {
        Text(Title()).Write(x, y + DeltaTextt(), width);

        ColorDraw(false).SetAsCurrent();

        Text(CurrentChoice()).Write(x + width + 10, y + DeltaTextt());
    }
    else
    {
        Text(CurrentChoice()).Write(x, y + DeltaTextt(), width);
    }
}


Color Item::ColorBackground(bool selected)
{
    return selected ? Color::MENU_SELECT : Color::MENU_UNSELECT;
}


Color Item::ColorDraw(bool selected)
{
    return selected ? Color::MENU_UNSELECT : Color::MENU_SELECT;
}


pchar Button::GetTitle() const
{
    return text;
}


void Button::SetTitle(pchar ru)
{
    text = ru;
}


pchar Choice::CurrentChoice() const
{
    return names[*state];
}


pchar Choice::Title() const
{
    return title;
}


pchar Parameter::Title() const
{
    return title;
}


Value VParameter::GetCalibrateValue(TypePicture::E type, VoltageMode::E mode)
{
    Display::value_in = GetValue();

    const SettingsCal::StructCal &cal = gset.cal.cal[type][mode][0];

    float voltage = (std::fabsf(GetValue().ToUnits()) - cal.offset) / cal.k;

    Value result((int)(voltage * 1e3f), TypeValue::Voltage);

    Display::value_out = result;

    return result;
}


bool Button::OnKey(Key::E key)
{
    if (key == Key::OK || key == Key::GovButton)
    {
        if (funcOnPress)
        {
            funcOnPress();

            Beeper::ButtonPress();

            return true;
        }
    }

    return false;
}


bool Choice::OnKey(Key::E key)
{
    if (key == Key::OK || key == Key::GovButton)
    {
        *state = (uint8)((*state) + 1);

        if (*state == NumStates())
        {
            *state = 0;
        }

        if (funcOnPress)
        {
            funcOnPress();
        }

        return true;
    }

    return false;
}


bool Parameter::OnKey(Key::E key)
{
    if (key == Key::OK ||
        key == Key::GovButton ||
        (key >= Key::_0 && key <= Key::_9))
    {
        if (IsNowEdited())
        {
            if (key == Key::OK || key == Key::GovButton)
            {
                editable = nullptr;

                Value new_value(0);

                ds.ToValue(&new_value);

                const int milli_units = new_value.ToMU();

                if (milli_units < GetMin().ToMU())
                {
                    GetValue() = Value(GetMin().GetRaw());
                }
                else if (milli_units > GetMax().ToMU())
                {
                    GetValue() = Value(GetMax().GetRaw());
                }
                else
                {
                    GetValue() = new_value;
                }

                return true;
            }
        }
        else
        {
            editable = this;

            ds.Set(this);

            if (key >= Key::_0 && key <= Key::_9)
            {
                ds.PressKey(Key::Esc);
                ds.PressKey(key);
            }

            return true;
        }
    }

    if (IsNowEdited())
    {
        ds.PressKey(key);

        return true;
    }

    return false;
}


bool Page::OnKey(Key::E key)
{
    if (key == Key::Left || key == Key::GovLeft)
    {
        if (!Parameter::editable)
        {
            Math::CircleIncrease<int>(&selectedItem, 0, NumItems() - 1);

            current = SelectedItem()->IsParameter() ? (Parameter *)SelectedItem() : nullptr;

            return true;
        }
    }
    else if (key == Key::Right || key == Key::GovRight)
    {
        if (!Parameter::editable)
        {
            Math::CircleDecrease<int>(&selectedItem, 0, NumItems() - 1);

            current = SelectedItem()->IsParameter() ? (Parameter *)SelectedItem() : nullptr;

            return true;
        }
    }

    return false;
}


Value &Parameter::GetValue() const
{
    return *value[VoltageMode::Current()];
}


const Value &Parameter::GetMin() const
{
    return VoltageMode::Is12() ? min12 : min24;
}


const Value &Parameter::GetMax() const
{
    return VoltageMode::Is12() ? max12 : max24;
}


bool Parameter::IsNowSelected() const
{
    Page *p = Menu::OpenedPage();
    Parameter *s = p->GetCurrentParameter();

    return s == this;
}


bool Page::StartTest() const
{
    if (func_start_test)
    {
        return func_start_test();
    }

    return true;
}


bool Page::IsSignal(Page *page)
{
    return
        page == PageSignal1::self ||
        page == PageSignal2a::self ||
        page == PageSignal2b::self ||
        page == PageSignal3a::self ||
        page == PageSignal3b::self ||
        page == PageSignal4::self ||
        page == PageSignal5a::self ||
        page == PageSignal5b::self;
}


void Page::DrawConnectionServer()
{
    int x = Display::xConstParameters;

    int y = Display::yConstParameters - 25;

    Text("IT6523C : ").Write(x, y, Color::WHITE);

    x += 90;

    if (ServerTCP::IsConnected())
    {
        Text("ПОДКЛЮЧЕН").Write(x, y, Color::GREEN);
    }
    else
    {
        if ((TIME_MS / 500) % 2)
        {
            Text("НЕ ПОДКЛЮЧЕН").Write(x, y, Color::RED);
        }
    }
}


void FieldIP::Draw(int x, int y, int widht, bool selected /* = false */)
{
    if (IsOpened())
    {
        Button::Draw(x, y, widht, selected);

        x += 130;

        Rect(150, 30).FillRounded(x, y - 6, 1, Color::BACK, Color::WHITE);

        Text(buffer).Write(x + 5, y + 2, Color::WHITE);
    }
    else
    {
        Button::Draw(x, y, widht, selected);
    }
}


void FieldMAC::Draw(int x, int y, int widht, bool selected /* = false */)
{
    if (IsOpened())
    {
        Button::Draw(x, y, widht, selected);

        x += 130;

        Rect(150, 30).FillRounded(x, y - 6, 1, Color::BACK, Color::WHITE);

        Text(buffer).Write(x + 5, y + 2, Color::WHITE);
    }
    else
    {
        Button::Draw(x, y, widht, selected);
    }
}


void FieldPort::Draw(int x, int y, int width, bool selected /* = false */)
{
    if (IsOpened())
    {
        Button::Draw(x, y, width, selected);

        x += 130;

        Rect(150, 30).FillRounded(x, y - 6, 1, Color::BACK, Color::WHITE);

        Text(buffer).Write(x + 5, y + 2, Color::WHITE);
    }
    else
    {
        Button::Draw(x, y, width, selected);
    }
}


bool FieldIP::OnKey(Key::E key)
{
    if (IsOpened())
    {
        if (key == Key::OK)
        {
            uint8 bytes[4];

            if (ConvertStringToAddress(bytes))
            {
                std::memcpy(address, bytes, 4);
            }

            Close();
        }
        else if (key == Key::Esc)
        {
            Close();
        }
        else if (key == Key::Dot || (key >= Key::_0 && key <= Key::_9))
        {
            if (!edited)
            {
                buffer[0] = '\0';
            }

            edited = true;

            if (std::strlen(buffer) < 15)
            {
                std::strcat(buffer, Key::Name(key));
            }
        }

        return true;
    }
    else
    {
        if (key == Key::OK)
        {
            Open();

            return true;
        }
    }

    return false;
}


bool FieldMAC::OnKey(Key::E key)
{
    if (IsOpened())
    {
        if (key == Key::OK)
        {
            uint8 bytes[6];

            if (ConvertStringToMAC(bytes))
            {
                std::memcpy(mac, bytes, 6);
            }

            Close();
        }
        else if (key == Key::Esc)
        {
            Close();
        }
        else if (key == Key::Dot || (key >= Key::_0 && key <= Key::_9))
        {
            if (!edited)
            {
                buffer[0] = '\0';
            }

            edited = true;

            if (std::strlen(buffer) < 17)
            {
                std::strcat(buffer, (key == Key::Dot) ? ":" : Key::Name(key));
            }
        }

        return true;
    }
    else
    {
        if (key == Key::OK)
        {
            Open();

            return true;
        }
    }

    return false;
}


bool FieldPort::OnKey(Key::E key)
{
    if (IsOpened())
    {
        if (key == Key::OK)
        {
            int value = std::atoi(buffer);

            if (value > 0 && value < 65536)
            {
                *port = (uint16)value;
            }

            Close();
        }
        else if (key == Key::Esc)
        {
            Close();
        }
        else if (key >= Key::_0 && key <= Key::_9)
        {
            if (!edited)
            {
                buffer[0] = '\0';
            }

            edited = true;

            if (std::strlen(buffer) < 5)
            {
                std::strcat(buffer, Key::Name(key));
            }
        }

        return true;
    }
    else
    {
        if (key == Key::OK)
        {
            Open();

            return true;
        }
    }

    return false;
}


bool FieldIP::ConvertStringToAddress(uint8 bytes[4]) const
{
    uint length = std::strlen(buffer);

    int counter = 0;

    for (uint i = 0; i < length; i++)
    {
        if (buffer[i] == '.')
        {
            counter++;
        }
    }

    if (counter != 3)
    {
        return false;
    }

    const char *pointer = buffer;

    for (uint i = 0; i < 4; i++)
    {
        const char *start = pointer;

        while (*pointer != '.' && *pointer != '\0')
        {
            pointer++;
        }

        // Нашли конец числа

        const char *end = pointer;

        pointer++;

        char str[10] = { '\0' };

        while (start != end)
        {
            char symbol[2] = { *start, '\0' };
            std::strcat(str, symbol);
            start++;
        }

        int value = 0;
        char *e = nullptr;

        SU::String2Int(str, &value, &e);

        if (value > 255)
        {
            return false;
        }

        bytes[i] = (uint8)value;
    }

    return true;
}


bool FieldMAC::ConvertStringToMAC(uint8 bytes[6]) const
{
    uint length = std::strlen(buffer);

    int counter = 0;

    for (uint i = 0; i < length; i++)
    {
        if (buffer[i] == ':')
        {
            counter++;
        }
    }

    if (counter != 5)
    {
        return false;
    }

    const char *pointer = buffer;

    for (uint i = 0; i < 6; i++)
    {
        const char *start = pointer;

        while (*pointer != ':' && *pointer != '\0')
        {
            pointer++;
        }

        // Нашли конец числа

        const char *end = pointer;

        pointer++;

        char str[10] = { '\0' };

        while (start != end)
        {
            char symbol[2] = { *start, '\0' };
            std::strcat(str, symbol);
            start++;
        }

        char *e = nullptr;

        int value = std::strtol(str, &e, 16);

        if (value > 255 || *e != '\0')
        {
            return false;
        }

        bytes[i] = (uint8)value;
    }

    return true;
}


void FieldIP::Open()
{
    std::strcpy(buffer, Text("%d.%d.%d.%d", address[0], address[1], address[2], address[3]).c_str());

    edited = false;

    Button::Open();
}


void FieldMAC::Open()
{
    std::strcpy(buffer, Text("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]).c_str());

    edited = false;

    Button::Open();
}


void FieldPort::Open()
{
    std::strcpy(buffer, Text("%u", *port).c_str());

    edited = false;

    Button::Open();
}


void FieldIP::Close()
{
    Button::Close();
}


void FieldMAC::Close()
{
    Button::Close();
}


void FieldPort::Close()
{
    Button::Close();
}


VParameter *VParameter::CurrentUs()
{
    Page *p = Menu::OpenedPage();

    if (p == PageSignal1::self)
    {
        return &PageSignal1::param_Us;
    }

    return nullptr;
}
