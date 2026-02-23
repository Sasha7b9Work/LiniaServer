// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


struct BaseMessage;


// Структура содержит данные для элементарного задания, передаваемого в Interface
struct Task
{
    // Этот конструктор может использоваться только в Interface для сравнения
    Task(BaseMessage *msg);

    Task(BaseMessage *msg, bool (*process)(BaseMessage *), bool (*equal)(Task *, Task *));
    // Деструктор. В нём нужно удалить сообщение
    ~Task();
    // Функция сравнения
    bool Equals(Task *, Task *);
    // Соообщение для пересылки
    BaseMessage *message;
    // Возвращает указатель на готовое к использованию (со сброщенным указателем) сообщение
    BaseMessage *GetMessage();
    // Послать сообщение задания
    void TransmitMessage();
    // Возвращает true, если прошло слишком мало времени после предыдущей засылки сообщения
    bool PassedLittleTimeAfterSend();
    // Функция обработки ответа
    bool(*funcProcess)(BaseMessage *);
private:
    // Время последней передачи сообщения
    uint timeLast;
    // Функция сравнения
    bool (*funcEqual)(Task *, Task *);
};



namespace PInterface
{
    // Для приёма сообщений от устройства
    void Update();

    // Добавляеет задание. Если не получилось, возвращает false
    void AddTask(Task *);

    void AddMessageForTransmit(BaseMessage *);
};
