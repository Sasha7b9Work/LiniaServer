// 2025/02/20 11:40:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "defines.h"


/*
    Ждёт входящих подключений
*/

namespace ClientTCP
{
    void Init(void (*callbackReceive)(pchar, uint));

    void SendBuffer(pchar buffer, uint length);

    // В конце строки передаётся "\r\n"
    void SendString(char *format, ...);

    // Строка передаётся "как есть", без добавления завершающих символов
    void SendStringRAW(char *foramt, ...);

    bool IsConnected();
}
