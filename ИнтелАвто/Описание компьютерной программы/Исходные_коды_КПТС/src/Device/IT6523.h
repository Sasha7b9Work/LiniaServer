// 2025/02/27 10:37:09 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Settings/Settings.h"


namespace IT6523
{
    void Update();

    // Запускает процесс формирования периодических импульсов, формируемых внешним генератором
    // Возвращает false, если запуск не может быть произведён - не прошло достаточно времени
    bool Start(TypePicture::E, int num_pulses);

    // Включает соотвествующее напряжение на внешнем генераторе
    bool Start(const Value &current);

    // Приостановить процесс формирования импульсов
    void Pause();

    // Продолжить процесс формирования импульсов
    void Resume();

    // Остановить процесс формирования импульсов
    void Stop();

    // Само дополнит завершающими символами
    void SendCommand(pchar);

    void SendCommandF(pchar, ...);

    // Здесь происходит очередной импульс
    void CallbackOnTimerImpulse();

    // Столько импульсов осталось произвести
    int RemainedPulses();

    // Осталось до запуска следующего "тяжёлого" импульса
    uint TimeLeftToHeavyImpulse();
}
