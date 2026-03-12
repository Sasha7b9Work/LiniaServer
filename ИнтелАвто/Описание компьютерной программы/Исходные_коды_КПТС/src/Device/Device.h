// 2024/02/01 10:15:57 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


namespace Device
{
    // По нажатию на СТАРТ вызываем
    bool Start();

    // Это тоже вызывается при нажатии на СТАРТ, но когда тест в процессе выполнения
    void Pause();

    // Это вызывается при нажатии на СТАРТ, когда находимся в паузе
    void Resume();

    // По нажатию на СТОП вызываем
    void Stop();

    // Тест в состоянии выполнения
    bool IsRunning();

    // Тест в паузе
    bool InPause();

    // Тест не выполняется
    bool IsStopped();

    void TasksUpdate();
}
