// 2025/03/25 09:30:09 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Menu/Pages/Pages.h"
#include "Menu/Menu.h"
#include "Menu/MenuItemsDef.h"
#include "Display/Text_.h"
#include "Hardware/Keyboard/Keyboard_.h"
#include "Display/Display_.h"
#include "Hardware/Timer.h"
#include "Menu/Pages/InputField.h"
#include "Connector/Device/Messages_.h"
#include <cmath>


using namespace Primitives;


namespace PageCalibration
{
//    static const int INVALID_VOLTAGE = 999999999;
    static const uint TIME_TIMER = 5000;

    static const int NUM_SIGNALS = 4;       // Калибруем только четыре из всех сигналов - те, которые формируем сами
    static uint8 type_signal = 0;           // Один из 
    static TypePicture::E CurrentSignal();

    static const int NUM_ACCUM = 2;
    static uint8 type_accum = 0;

    static const int NUM_POINTS = 2;        // Столько всего точек для калибровки
    static int  current_point = 0;          // Точка, которую сейчас измеряем
                                            // 0 - минимальное напряжение
                                            // NUM_POINTS - 1 - максимальное напряжение

    static bool output_en = false;

    // Здесь измеренные значения, введённые вручную
    static Voltage values[NUM_SIGNALS][NUM_ACCUM][NUM_POINTS] =
    {
        {                               // 1
            { Voltage(0), Voltage(0) },
            { Voltage(0), Voltage(0) }
        },
        {                               // 2a
            { Voltage(0), Voltage(0) },
            { Voltage(0), Voltage(0) }
        },
        {                               // 3a
            { Voltage(0), Voltage(0) },
            { Voltage(0), Voltage(0) }
        },
        {                               // 3b
            { Voltage(0), Voltage(0) },
            { Voltage(0), Voltage(0) }
        }
    };

    // Поле для ввода измеренного значения
    static InputField field;

    // Подать напряжение на выход в соответствии с установленными параметрами
    static void EnableOutput();

    // Снять напряжение с выхода
    static void DisableOutput();

    // Возвращает напряжение текущей точки в вольтах
    static int GetVoltagePoint(int num_point);

    // Отказаться от применения калибровочных факторов
    static void RefuseCalibrateFactors();

    // Рассчитать и приименить калибровочные факторы
    static void CalculateCalibrateFactors();

    static void TimerFunction();

    struct State
    {
        enum E
        {
            Normal,                     // Начальное состояние
            EnableOutputAndWaitEnter,   // Установлено напряжение на выходе и ожидается ввод измеренного значения
            ConfirmForSave,             // Вывести подтверждение на сохранение настроек
            FactorSave,                 // Калибровочный коэффициент сохранён
            RefuseCalibration           // Калибровочный коэффициент не сохранён
        };
    };

    static bool FieldIsVisible();

    static State::E state = State::Normal;

    static void FuncPress_Back()
    {
        Menu::SetOpenedPage(PageMain::self);
    }

    DEF_BUTTON(bBack,
        STR_BACK,
        FuncPress_Back
    );

    DEF_CHOICE_4_FULL(chTypeSignal,
        "Импульс",
        "1", "2a", "3a", "3b",
        type_signal,
        FuncVV
    );

    DEF_CHOICE_2_FULL(chTypeAccum,
        "Аккумулятор",
        "12В", "24В",
        type_accum,
        FuncVV
    );

    static void FuncPress_Calibrate()
    {
        if (state == State::Normal)                     // Исходное состояние
        {
            state = State::EnableOutputAndWaitEnter;    // После нажатия кнопки будем ждать ввода измеренного значения

            current_point = 0;                          // Будем устанавливать минимальное напряжение

            EnableOutput();                             // Подаём напряжение на выход


        }
        else if (state == State::EnableOutputAndWaitEnter)
        {
            if (field.GetValueMilliUnits() != 0)
            {
                DisableOutput();

                values[type_signal][type_accum][current_point].Set(field.GetValueMilliUnits());

                if (current_point < NUM_POINTS - 1)
                {
                    current_point++;

                    EnableOutput();
                }
                else
                {
                    state = State::ConfirmForSave;      // Если все точки пройдены, то переходим в состояние ожидания сохранения
                }

                field.Reset();
            }
        }
        else if (state == State::ConfirmForSave)
        {

        }
        else if (state == State::FactorSave)
        {

        }
        else if (state == State::RefuseCalibration)
        {

        }
    }

    DEF_BUTTON(bCalibrate,
        "Калибровать",
        FuncPress_Calibrate
    );

    static void FuncPress_Reset()
    {
        
    }

    DEF_BUTTON(bReset,
        "Сброс",
        FuncPress_Reset
    );

    static Item *items[] =
    {
        &bBack,
        &chTypeSignal,
        &chTypeAccum,
        &bCalibrate,
//        &bReset,
        nullptr
    };

    static void FuncDraw()
    {
        int dy = 25;

        if (state == State::Normal)
        {
            int x = 180;
            int y = 70;

            Text("Для начала калибровки").Write(x + 30, y, Color::WHITE);

            y += 2 * dy;

            Text("нажмите кнопку \"Калибровать\".").Write(x, y);

            y = 180;

            x -= 30;

            Text("k = %.3f", (double)gset.cal.cal[CurrentSignal()][type_accum][0].k).Write(x, y, Display::Width() - x); y += 30;

            Text("offset = %.3f В", (double)gset.cal.cal[CurrentSignal()][type_accum][0].offset).Write(x, y, Display::Width() - x);

        }
        else if (state == State::EnableOutputAndWaitEnter)
        {
            Text("Точка %d", current_point + 1).Write(260, 30, Color::WHITE);

            Text("Введите значение измеренного").Write(200, 70);

            Text("напряжения").Write(200, 100);

            field.Draw(240, 150, FieldIsVisible());
        }
        else if (state == State::ConfirmForSave)
        {
            for (int i = 0; i < NUM_POINTS; i++)
            {
                int y = 20 + 25 * i;

                Text("Точка %d", i + 1).Write(200, y, Color::WHITE);

                values[type_signal][type_accum][i].Draw(300, y);
            }

            int x = 170;
            int y = 125;

            Text("Для сохранения калибровочных").Write(x, y);
            Text("коэффициентов нажмите OK.").Write(x, y + 25);

            Text("Для отмены нажмите Esc.").Write(x, y + 75);
        }
        else if (state == State::FactorSave)
        {
            int x = 190;
            int y = 70;

            Text("Коэффициент").Write(x, y, Display::Width() - x, Color::GREEN); y += 30;

            Text("cохранен").Write(x, y, Display::Width() - x); y += 30;

            Text("k = %.3f", (double)gset.cal.cal[CurrentSignal()][type_accum][0].k).Write(x, y, Display::Width() - x); y += 30;

            Text("offset = %.3f В", (double)gset.cal.cal[CurrentSignal()][type_accum][0].offset).Write(x, y, Display::Width() - x); y += 30;
        }
        else if (state == State::RefuseCalibration)
        {
            int x = 180;
            int y = 70;

            Text("Коэффициент").Write(x, y, Display::Width() - x, Color::RED); y += 30;

            Text("не cохранен").Write(x, y, Display::Width() - x); x += 30;

            Text("k = %.3f", (double)gset.cal.cal[CurrentSignal()][type_accum][0].k).Write(x, y, Display::Width() - x); y += 30;

            Text("offset = %.3f В", (double)gset.cal.cal[CurrentSignal()][type_accum][0].offset).Write(x, y, Display::Width() - x); y += 30;
        }

        Font::Set(TypeFont::GOSTB28B);

        Text("Выход:").Write(30, 150, Color::WHITE);

        if (output_en)
        {
            Text buffer("%d", GetVoltagePoint(current_point));

            Text("%s B", buffer.c_str()).Write(45, 205, Color::WHITE);
        }
        else
        {
            Text("ОТКЛ").Write(40, 205, Color::RED);
        }

        Font::Set(TypeFont::GOSTAU16BOLD);
    }

    static Page page(items, FuncDraw, nullptr);

    Page *self = &page;


    void UpdateInput()
    {
        while (!Keyboard::Empty())
        {
            const Key::E key = Keyboard::NextKey();

            if (key == Key::Start || key == Key::Stop)
            {
            }
            else if (FieldIsVisible() && ((key >= Key::_0 && key <= Key::_9) || key == Key::Dot || key == Key::Esc))
            {
                field.OnKey(key);
            }
            else if (state == State::ConfirmForSave)                        // Ожидание подтверждения калибровки
            {
                if (key == Key::OK)
                {
                    CalculateCalibrateFactors();
                }
                else if (key == Key::Esc)
                {
                    RefuseCalibrateFactors();
                }
            }
            else if (state == State::FactorSave || state == State::RefuseCalibration)
            {
            }
            else
            {
                if (state == State::Normal || key == Key::OK)
                {
                    Menu::Input::OnKey(key);
                }
            }
        }
    }
}


void PageCalibration::EnableOutput()
{
    Voltage voltage(GetVoltagePoint(current_point) * 1000);
    Time time(500);

    if (CurrentSignal() == TypePicture::_1)
    {
        if (type_accum == 0)
        {
            Message::Start1_12V(voltage, time).Transmit();
        }
        else
        {
            Message::Start1_24V(voltage, time).Transmit();
        }
    }
    else if (CurrentSignal() == TypePicture::_2a)
    {
        Message::Start2A(voltage, time).Transmit();
    }
    else if (CurrentSignal() == TypePicture::_3a)
    {
        Message::Start3A(voltage).Transmit();
    }
    else if (CurrentSignal() == TypePicture::_3b)
    {
        Message::Start3B(voltage).Transmit();
    }

    output_en = true;
}


void PageCalibration::DisableOutput()
{
    Message::Stop().Transmit();

    output_en = false;
}


void PageCalibration::TimerFunction()
{
    state = State::Normal;
}


int PageCalibration::GetVoltagePoint(int num_point)
{
    static const int voltages[NUM_SIGNALS][NUM_ACCUM][NUM_POINTS] =
    {
        {                                                                           // 1
            { PageSignal1::us12_min, PageSignal1::us12_max },
            { PageSignal1::us24_min, PageSignal1::us24_max }
        },
        {                                                                           // 2a
            { PageSignal2a::us12_min, PageSignal2a::us12_max },
            { PageSignal2a::us24_min, PageSignal2a::us24_max }
        },
        {                                                                           // 3a
            { PageSignal3a::us12_min, PageSignal3a::us12_max },
            { PageSignal3a::us24_min, PageSignal3a::us24_max }
        },
        {                                                                           // 3b
            { PageSignal3b::us12_min, PageSignal3b::us12_max },
            { PageSignal3b::us24_min, PageSignal3b::us24_max }
        }
    };

    return voltages[type_signal][type_accum][num_point];
}


bool PageCalibration::FieldIsVisible()
{
    return state == State::EnableOutputAndWaitEnter;
}


void PageCalibration::CalculateCalibrateFactors()
{
    /*
          Выход
            |
        Uo2 |..................+
            |                  .
            |                  .
            |                  .
        Uo1 |.....+            .
            |     .            .
            +----------------------- Вход
                 Ui1          Ui2
    
        Uo = offset + Ui * k
    
        k = (Uo2 - Uo1) / (Ui2 - Ui1)
    
        offset = Uo1 - Ui1 * k
    */

    float out1 = values[type_signal][type_accum][0].ToUnits();
    float out2 = values[type_signal][type_accum][1].ToUnits();

    float in1 = std::fabsf((float)GetVoltagePoint(0));
    float in2 = std::fabsf((float)GetVoltagePoint(1));

    float k = (out2 - out1) / (in2 - in1);
    float offset = out1 - in1 * k;

    gset.cal.cal[CurrentSignal()][type_accum][0].k = k;
    gset.cal.cal[CurrentSignal()][type_accum][0].offset = offset;

    state = State::FactorSave;

    Timer::SetDefferedOnceTask(TimerTask::Calibrate, TIME_TIMER, TimerFunction);
}


void PageCalibration::RefuseCalibrateFactors()
{
    state = State::RefuseCalibration;

    Timer::SetDefferedOnceTask(TimerTask::Calibrate, TIME_TIMER, TimerFunction);
}


TypePicture::E PageCalibration::CurrentSignal()
{
    static TypePicture::E signals[4] =
    {
        TypePicture::_1,
        TypePicture::_2a,
        TypePicture::_3a,
        TypePicture::_3b
    };

    return signals[type_signal];
}
