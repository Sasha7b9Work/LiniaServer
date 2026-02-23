// 2025/02/13 10:50:04 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/ParameterDrawStruct.h"
#include "Hardware/Keyboard/Keyboard_.h"
#include "Menu/MenuItems.h"
#include "Display/Primitives_.h"
#include "Display/Text_.h"
#include "Utils/Math_.h"
#include "Utils/StringUtils_.h"
#include <cstring>


using namespace Primitives;


void ParameterDrawStruct::PressKey(int _key)
{
    Key::E key = (Key::E)_key;

    if (key == Key::Minus)
    {
        if (parameter->GetValue().IsVoltage() && p.index == 0)
        {
            p.SetSymbolToCurrentPos('-');
        }
    }
    else if (key == Key::Left)
    {
        if (p.index > 0)
        {
            p.index--;
        }
    }
    else if (key == Key::Right)
    {
        if (p.index < (int)std::strlen(p.symbols) - 1)
        {
            p.index++;
        }
    }
    else if (key == Key::Esc)
    {
        p.symbols[0] = '\0';
        p.index = 0;
    }
    else if ((key >= Key::_0 && key <= Key::_9) || key == Key::Dot)
    {
        static const char _keys[Key::Count] = { ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', ',' };

        p.SetSymbolToCurrentPos(_keys[key]);
    }
    else if (key == Key::GovLeft)
    {
        DecreaseInPosition(p.index);
    }
    else if (key == Key::GovRight)
    {
        IncreaseInPosition(p.index);
    }
}


void ParameterDrawStruct::Draw(int x, int y) const
{
    char buffer[128] = { '\0' };

    std::strcat(buffer, p.symbols);

    int pos_hight = p.index;                            // Позиция подсвеченного разряда

    int size_buffer = (int)std::strlen(buffer);

    for (int i = 0; i < size_buffer; i++)
    {
        Rect(13, 18).Fill(x + 10 * i - 2, y - 2, (i == pos_hight) ? Color::BLUE : Color::BACK);

        char text[2] = { buffer[i], '\0' };
        Text(text).Write(x + 10 * i, y, Color::WHITE);
    }

    if (p.value.IsVoltage())
    {

    }
}


void ParameterDrawStruct::Set(Parameter *_param)
{
    parameter = _param;

    p.Set(parameter->GetValue());
}


void ParameterDrawStruct::Params::Set(const Value &val)
{
    value = val;

    if (value.IsRaw())
    {
        std::sprintf(symbols, "%u", value.GetRaw());
    }
    else
    {
        std::sprintf(symbols, "%d,", value.WholePart());

        char buffer[4];

        std::strcat(symbols, value.FractPart(buffer));
    }

    SetIndexInLastSignedSymbol();
}


void ParameterDrawStruct::Params::SetIndexInLastSignedSymbol()
{
    index = (int)std::strlen(symbols) - 1;

    while (true)
    {
        if (symbols[index] == '0')
        {
            index--;
        }
        else if (symbols[index] == ',')
        {
            index--;
        }
        else if (symbols[index + 1] == ',')
        {
            break;
        }
        else
        {
            break;
        }
        index--;
    }
}


bool ParameterDrawStruct::IsMinimalValueOrLess() const
{
    Value value(0);

    ToValue(&value);

    int int_value = value.ToMU();
    int min_value = parameter->GetMin().ToMU();

    return int_value <= min_value;
}


bool ParameterDrawStruct::IsMaximumValueOrAbove() const
{
    Value value(0);

    ToValue(&value);

    int int_value = value.ToMU();
    int max_value = parameter->GetMax().ToMU();

    return int_value >= max_value;
}


void ParameterDrawStruct::ToValue(Value *result) const
{
    char buffer[128];

    std::strcpy(buffer, p.symbols);

    char *end1 = nullptr;

    int whole_value = 0;

    SU::String2Int(buffer, &whole_value, &end1);

    int fract_value = 0;

    char *end2 = nullptr;

    SU::String2Int(end1 + 1, &fract_value, &end2);

    const int diff_end = end2 - end1;   // Итоговое количество миллиюнитов зависит от того, сколько всего символов в дробной части

    if (diff_end == 2)
    {
        fract_value *= 100;
    }
    else if (diff_end == 3)
    {
        fract_value *= 10;
    }

    uint raw = (uint)(whole_value * 1000 + fract_value);

    if (parameter->GetValue().GetType() == TypeValue::Time)
    {
        raw |= (1 << 29);
        raw |= (1 << 30);
    }
    else if (parameter->GetValue().GetType() == TypeValue::Voltage)
    {
        raw |= (1 << 29);
    }
    else if (parameter->GetValue().GetType() == TypeValue::Current)
    {
        raw |= (1 << 30);
    }
    else
    {
        raw /= 1000;
    }

    *result = Value(raw);
}


void ParameterDrawStruct::Params::SetSymbolToCurrentPos(char symbol)
{
    if (symbol == ',' && SU::FindPosition(symbols, ',') != -1)
    {
        return;
    }

    if (index < SIZE_BUFER - 1)
    {
        if (index >= NumSymbols())
        {
            int pos = SU::FindPosition(symbols, ',');

            if (pos < 0 || (int)std::strlen(symbols) - pos < 4)
            {
                symbols[index++] = symbol;
                symbols[index] = '\0';
            }
        }
        else
        {
            if (symbols[index] == ',' && symbol >= '0' && symbol <= '9')
            {
                index++;
                return;
            }
            else if (symbol == ',' && SU::FindPosition(symbols, ',') != -1)
            {

            }
            else 
            {
                symbols[index] = symbol;
            }
            index++;
        }
    }
}


int ParameterDrawStruct::Params::NumSymbols() const
{
    return (int)std::strlen(symbols);
}


void ParameterDrawStruct::IncreaseInPosition(int pos)
{
    if (IsMaximumValueOrAbove())
    {
        return;
    }

    p.IncreaseInPosition(pos);
}


void ParameterDrawStruct::Params::IncreaseInPosition(int pos)
{
    char symbol = symbols[pos];

    if (symbol < '0' || symbol > '9')
    {
        return;
    }

    symbol++;

    if (symbol > '9')
    {
        if (OnLeftAllNines(pos))
        {
            bool first_already_one = false;         // Когда установим первый символ в "1", сделаем это true

            for (int i = 0; i < pos; i++)
            {
                if (IsDigit(symbols[i]))
                {
                    symbols[i] = first_already_one ? '0' : '1';

                    first_already_one = true;
                }
            }

            symbol = '0';

            char buffer[2] = { '0', '\0' };
            std::strcat(symbols, buffer);
            index++;
        }
        else
        {
            symbol = '0';
            IncreaseInPosition(pos - 1);
        }
    }

    symbols[pos] = symbol;

}


void ParameterDrawStruct::DecreaseInPosition(int pos)
{
    if (IsMinimalValueOrLess())
    {
        return;
    }

    p.DecreaseInPosition(pos);
}


void ParameterDrawStruct::Params::DecreaseInPosition(int pos)
{
    char symbol = symbols[pos];

    if (!IsDigit(symbol))
    {
        return;
    }

    symbol--;

    if (symbol < '0')
    {
        symbol = '9';
        DecreaseInPosition(pos - 1);
    }

    symbols[pos] = symbol;
}


bool ParameterDrawStruct::Params::OnLeftAllNines(int pos)
{
    for (int i = 0; i < pos; i++)
    {
        if (!IsDigit(symbols[i]))
        {
            continue;
        }

        if (symbols[i] != '9')
        {
            return false;
        }
    }

    return true;
}


bool ParameterDrawStruct::Params::IsDigit(char symbol) const
{
    return (symbol >= '0') && (symbol <= '9');
}


void Value::Draw(int x, int y) const
{
    char string[32];

    char *pointer = string;

    int value = ToMU();

    if (value < 0)
    {
        value = -value;
        string[0] = '-';
        pointer++;
    }

    *pointer = '\0';

    if (value < 1000 || IsRaw())
    {
        std::strcat(pointer, Text("%d", value).c_str());

        if (GetType() == TypeValue::Time)
        {
            std::strcat(string, "м");
        }
    }
    else
    {
        int int_value = value / 1000;

        std::strcat(pointer, Text("%d", int_value).c_str());

        std::strcat(string, ",");

        value = value - int_value * 1000;

        std::strcat(string, Text("%d", value).c_str());

        while (true)
        {
            if (std::strlen(string) < 2)
            {
                break;
            }

            if (string[std::strlen(string) - 1] != '0')
            {
                break;
            }

            if (string[std::strlen(string) - 2] == ',')
            {
                break;
            }

            string[std::strlen(string) - 1] = '\0';
        }
    }

    if (GetType() != TypeValue::Raw)
    {
        if (GetType() == TypeValue::Time)
        {
            std::strcat(string, "с");
        }
        else if (GetType() == TypeValue::Voltage)
        {
            std::strcat(string, "В");
        }
        else if (GetType() == TypeValue::Current)
        {
            std::strcat(string, "А");
        }
    }
    Text(string).Write(x, y, Color::WHITE);
}



/*
    *** Общие положения ***

    - index указывает на текущую позицию без учёта знака
    - если подсвечен разряд, в котором есть значение, то при нажатии цифровой кнопки в этом месте
      появляется нажатая цифра, при вращении ручки это значение увеличивается/уменьшается

    *** Алгоритмы ***

    1.  Открытие параметра
        Выводится целочисленное значение без единиц измерения
        Последний разряд подсвечен

    2.  Вращение ручки вниз.
        Уменьшает значение в текущем разряде, если величина установленного напряжения больше минимальной (допускает уменьшение)
*/
