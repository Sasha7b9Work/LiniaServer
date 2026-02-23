// 2024/02/05 11:50:16 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Hardware/HAL/HAL.h"
#include "Connector/Device/Value_.h"


#define TIME_MS HAL_TIM::TimeMS()


class Parameter;


struct TimerTask
{
    enum E
    {
        ChangeColorOnLabelStop,     // Изменить цвет кнопки СТОП через некоторое время после остановки
        USBD,
        StopSound,
        IT6523,
        Calibrate,                  // Функция страницы калибровки
        DisplayWarningMessage,
        Count
    };
};


namespace Timer
{
    // Однократное значение - выполняется только один раз
    void SetDefferedOnceTask(TimerTask::E, uint time, void (*func)());

    void SetPeriodicTask(TimerTask::E, uint time, void (*func)());

    void DisableTask(TimerTask::E);

    void UpdateTasks();

    void Delay(uint timeMS);
}


struct TimeMeterMS
{
    TimeMeterMS()
    {
        Reset();
    }

    void Reset()
    {
        time_reset = TIME_MS;
    }

    uint ElapsedTime()
    {
        return TIME_MS - time_reset;
    }
private:
    uint time_reset;
};


// Отсчитывает время до завершения работы теста
namespace RemainingTimeCounter
{
    // Запускает процесс отсчёта
    void Start(int period_ms, const Parameter &N);

    void Stop();

    // Возвращает в текстовом виде оставшееся время
    pchar RemainingTime(char buffer[128]);

    // Приостановить процесс отсчёта
    void Pause();

    // Продолжить процесс отсчёта
    void Resume();

    // Идёт отсчёт
    bool InProcess();
};
