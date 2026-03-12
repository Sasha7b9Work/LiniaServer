// 2023/11/15 14:58:11 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


struct BufferParser
{
    char data[128];

    uint ToUINT() const;
};


namespace Parser
{
    // Возвращает num-ое слово. Разделителями являются ':', ' ', ','. Если слово в кавычках, то кавычки опускаются
    // Нумерация начинается с 1
    pchar GetWord(pchar, int num, BufferParser *);
}
