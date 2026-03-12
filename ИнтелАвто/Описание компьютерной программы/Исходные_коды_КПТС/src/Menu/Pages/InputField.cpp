// 2025/04/07 10:05:25 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/Pages/InputField.h"
#include "Display/Primitives_.h"
#include "Display/Text_.h"
#include "Utils/StringUtils_.h"
#include <cstring>
#include <cstdlib>


using namespace Primitives;


InputField::InputField()
{
    Reset();
}


void InputField::Draw(int x, int y, bool visible)
{
    if (!visible)
    {
        return;
    }

    Rect(80, 30).FillRounded(x, y, 1, Color::BACK, Color::WHITE);

    Font::Set(TypeFont::GOSTB28B);

    Text(buffer).Write(x + 5, y + 2, Color::WHITE);

    Font::Set(TypeFont::GOSTAU16BOLD);
}


void InputField::OnKey(Key::E key)
{
    if (key == Key::Esc)
    {
        Reset();
    }
    else
    {
        if (buffer[0] == 0 && key == Key::_0)
        {
            return;
        }

        if (std::strlen(buffer) < SIZE_BUFFER - 1)
        {
            char symbol = (char)(key - 1) + 0x30;

            if (key == Key::Dot)
            {
                symbol = '.';
            }

            if (symbol == '.' && IsConsist('.'))
            {
            }
            else
            {
                buffer[std::strlen(buffer)] = symbol;
            }
        }
    }
}


void InputField::Reset()
{
    std::memset(buffer, 0, SIZE_BUFFER);
}


int InputField::GetValueMilliUnits() const
{
    int pos = SU::FindPosition(buffer, '.');

    if (pos == -1)
    {
        return std::atoi(buffer) * 1000;
    }

    char units[32];

    SU::GetWord(buffer, 0, pos, units);

    char milliunits[32];

    SU::GetWord(buffer, pos + 1, (int)std::strlen(buffer), milliunits);

    int milli = std::atoi(milliunits);

    int length = (int)std::strlen(milliunits);

    if (length == 0)
    {
        milli *= 1000;
    }
    else if (length == 1)
    {
        milli *= 100;
    }
    else if (length == 2)
    {
        milli *= 10;
    }

    return std::atoi(units) * 1000 + milli;
}


bool InputField::IsConsist(char symbol) const
{
    uint size = std::strlen(buffer);

    for (uint i = 0; i < size; i++)
    {
        if (buffer[i] == symbol)
        {
            return true;
        }
    }

    return false;
}
