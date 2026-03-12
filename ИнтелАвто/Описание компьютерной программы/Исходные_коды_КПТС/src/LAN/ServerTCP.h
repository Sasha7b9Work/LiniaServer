// 2025/02/20 11:45:01 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


/*
    Подключается к удалённому серверу
*/

namespace ServerTCP
{
    void Init(void (*callbackReceive)(pchar, uint));

    void SendString(pchar buffer);

    void SendBuffer(pchar buffer, uint length);

    bool IsConnected();

    // Вызывается постоянно из ETH, когда подключён шнур. Нужно, чтобы определить наличие подключения
    void CallbackOnRxETH();
}
