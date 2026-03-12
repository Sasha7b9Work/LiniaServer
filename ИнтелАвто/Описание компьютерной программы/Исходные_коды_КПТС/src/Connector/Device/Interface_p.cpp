// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/Timer.h"
#include "Hardware/HAL/HAL.h"
#include "Connector/Device/Interface_p.h"
#include "Connector/Device/Transceiver_.h"
#include "Connector/Device/Messages_.h"
#include "Utils/List_.h"
#include <cstdlib>


static List<Task> tasks;


namespace PInterface
{
    // Обрабатывает answer, если запрос на него есть в очереди заданий. Возвращает true, если это так
    bool ProcessTask(BaseMessage *answer);

    // Обрабатывает очередь заданий, засылая сообщения тех из них, которые необходимо заслать
    void SendTasks();

    // Обработать ответ на задание
    void RunAnswer(ListElement<Task> *element, BaseMessage *answer);
}


void PInterface::AddMessageForTransmit(BaseMessage *message)
{
    Transceiver::Transmit(message);
}


void PInterface::Update()
{
    static uint time = 0;

    if (TIME_MS - time < 100)
    {
        return;
    }

    SendTasks();
}


void PInterface::AddTask(Task *task)
{
    if(!tasks.IsMember(task))                   // Если задания ещё нет в очереди
    {
        task->TransmitMessage();                // То посылаем его сообщение
        tasks.Append(task);                     // и добавляем в очередь сообщений для повторной отправки
    }
}


void Task::TransmitMessage()
{
    message->Transmit();    // Посылаем сообщение
    timeLast = TIME_MS;     // запоминаем время посылки
}


void PInterface::SendTasks()
{
    ListElement<Task> *element = tasks.First();

    while (element)
    {
        Task *task = element->Get();

        if (!task->PassedLittleTimeAfterSend())
        {
            task->TransmitMessage();
        }

        element = element->Next();
    }
}


bool Task::PassedLittleTimeAfterSend()
{
    return (TIME_MS - timeLast) < 1000;
}


bool PInterface::ProcessTask(BaseMessage *answer)
{
    ListElement<Task> *element = tasks.First();

    Task taskAnswer(answer);

    while (element)
    {
        Task *task = element->Get();

        if (task->Equals(task, &taskAnswer))
        {
            RunAnswer(element, answer);
            return true;
        }

        element = element->Next();
    }


    return false;
}


void PInterface::RunAnswer(ListElement<Task> *element, BaseMessage *answer)
{
    element->Get()->funcProcess(answer);
    tasks.Remove(element->Get());
    delete element;
}


bool Task::Equals(Task *task1, Task *task2)
{
    return funcEqual(task1, task2);
}


Task::Task(BaseMessage *msg) : funcProcess(nullptr), timeLast(0), funcEqual(nullptr)
{
    message = msg->Clone();
}


Task::Task(BaseMessage *msg, bool(*process)(BaseMessage *), bool(*equal)(Task *, Task *)) :funcProcess(process), timeLast(0), funcEqual(equal)
{
    message = msg->Clone();
}


Task::~Task()
{
    delete message;
}


BaseMessage *Task::GetMessage()
{
    message->ResetPointer();
    return message;
}


void BaseMessage::Transmit()
{
    PInterface::AddMessageForTransmit(this);
}
