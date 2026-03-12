// 2023/09/08 21:53:53 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "defines.h"
#include "Settings/Settings.h"
#include "Menu/ParameterDrawStruct.h"
#include "Hardware/Keyboard/Keyboard_.h"
#include <cstring>


struct TypeMeasure;
class Switch;


class Item
{
    friend class Hint;
public:
    Item() = default;

    virtual ~Item() { }

    static int Height();

    // Функция отрисовки
    virtual void Draw(int x, int y, int width, bool selected = false) = 0;

    virtual bool IsParameter() const { return false; }

    // Функция обработки нажатия кнопки/ручки
    virtual bool OnKey(Key::E) { return false; }

    Color ColorBackground(bool selected);

    static Color ColorDraw(bool selected);

    bool IsOpened() const
    {
        return is_opened;
    }

    virtual void Open()
    {
        is_opened = true;
    }

    virtual void Close()
    {
        is_opened = false;
    }

    void SetActive(bool active)
    {
        is_active = active;
    }

    bool IsActive() const
    {
        return is_active;
    }

protected:

    int DeltaTextt() const { return 3; }

    bool is_opened = false;

    bool is_active = true;
};


// Общего назначения
class Button : public Item
{
public:
    Button(pchar text_ru, void (*funcPress)()) :
        Item(), funcOnPress(funcPress)
    {
        text = text_ru;
    }
    virtual void Draw(int x, int y, int width, bool selected = false) override;
    virtual bool OnKey(Key::E) override;
    pchar GetTitle() const;
    void SetTitle(pchar);
private:
    pchar text;
    void (*funcOnPress)();
};


// Поле для ввода настройки сети - адреса, порта или MAC
class FieldIP : public Button
{
public:
    FieldIP(pchar text_ru, uint8 *_address) :
        Button(text_ru, [](){}), address(_address) { }

    virtual void Draw(int x, int y, int widht, bool selected = false) override;
    virtual bool OnKey(Key::E) override;

    virtual void Open() override;
    virtual void Close() override;

    bool ConvertStringToAddress(uint8[4]) const;

private:

    uint8 *address;
    static const int SIZE_BUFFER = 64;
    char buffer[SIZE_BUFFER];           // Здесь хранится в текстовом виде
    bool edited = false;                // Если true, то находимся в режие редактирования - уже идёт набор цифр
};


class FieldMAC : public Button
{
public:
    FieldMAC(pchar text_ru, uint8 *_mac) :
        Button(text_ru, []()
    {}), mac(_mac)
    {
    }

    virtual void Draw(int x, int y, int widht, bool selected = false) override;
    virtual bool OnKey(Key::E) override;

    virtual void Open() override;
    virtual void Close() override;

    bool ConvertStringToMAC(uint8[6]) const;

private:

    uint8 *mac;
    static const int SIZE_BUFFER = 64;
    char buffer[SIZE_BUFFER];           // Здесь хранится в текстовом виде
    bool edited = false;                // Если true, то находимся в режие редактирования - уже идёт набор цифр
};


class FieldPort : public Button
{
public:
    FieldPort(pchar title, uint16 *_port) :
        Button(title, []() {}), port(_port) { }

    virtual ~FieldPort() override { }

    virtual void Draw(int x, int y, int width, bool selected = false) override;
    virtual bool OnKey(Key::E) override;

    virtual void Open() override;
    virtual void Close() override;

private:
    uint16 *port;
    static const int SIZE_BUFFER = 64;
    char buffer[SIZE_BUFFER];
    bool edited = false;
};


// Общего назначения
class Choice : public Item
{
public:
    // Если title == nullptr, то имена будут выводиться прямо на кнопке, иначе - справа
    Choice(pchar _title, pchar *_names, void (*funcPress)(), uint8 *_state) :
        Item(), colorBack(Color::MENU_UNSELECT), title(_title), state(_state), funcOnPress(funcPress)
    {
        names = _names;
    }

    virtual void Draw(int x, int y, int width, bool selected = false) override;
    virtual bool OnKey(Key::E) override;
    pchar CurrentChoice() const;
    pchar Title() const;
    int Value() const { return (int)*state; }
    void SetColorBackground(const Color &color) { colorBack = color; }
    Color colorBack;        // Этим цветом будем отрисовывать фон в случае Choice для выбора цвета
private:
    pchar title;
    pchar *names;
    uint8 *state;
    void (*funcOnPress)();
    int NumStates() const;
};


class Parameter : public Item
{
public:
    Parameter(pchar title_ru,
        Value *value12, const Value &_min12, const Value &_max12,
        Value *value24, const Value &_min24, const Value &_max24,
        int _x, int _y) :
        Item(),
        min12(_min12), max12(_max12),
        min24(_min24), max24(_max24),
        x(_x), y(_y)
    {
        value[VoltageMode::_12] = value12;
        value[VoltageMode::_24] = value24;

        title = title_ru;
    }

    virtual void Draw(int x, int y, int width, bool selected = false) override;

    void Draw(int num_item) const;

    virtual bool OnKey(Key::E) override;

    pchar Title() const; 

    Value &GetValue() const;

    const Value &GetMin() const;
    const Value &GetMax() const;

    int X() const;
    int Y(int num_item) const;

    virtual bool IsParameter() const override { return true; }

    // Сейчас выделен
    bool IsNowSelected() const;

    // Сейчас редактируется
    bool IsNowEdited() const { return Parameter::editable == this; }

    // Редактируемый в данный момент
    static Parameter *editable;

private:

    pchar title;

    Value *value[VoltageMode::Count];

    const Value min12;
    const Value max12;

    const Value min24;
    const Value max24;

    int x = 0;
    int y = 0;

    ParameterDrawStruct ds;
};


class VParameter : public Parameter
{
public:
    VParameter(pchar title_ru,
        Value *value12, const Voltage &_min12, const Voltage &_max12,
        Value *value24, const Voltage &_min24, const Voltage &_max24,
        int _x, int _y) :
        Parameter(title_ru, value12, _min12, _max12, value24, _min24, _max24, _x, _y)
    {
    }

    Value GetCalibrateValue(TypePicture::E, VoltageMode::E);

    // Параметр Us для текущего сигнала
    static VParameter *CurrentUs();
};


class AParameter : public Parameter
{
public:
    AParameter(pchar title_ru,
        Value *value12, const Current &_min12, const Current &_max12,
        Value *value24, const Current &_min24, const Current &_max24,
        int _x = 0, int _y = 0) :
        Parameter(title_ru, value12, _min12, _max12, value24, _min24, _max24, _x, _y)
    {
    }
};


class TParameter : public Parameter
{
public:
    TParameter(pchar title_ru,
        Value *value12, const Time &_min12, const Time &_max12,
        Value *value24, const Time &_min24, const Time &_max24,
        int _x, int _y) :
        Parameter(title_ru, value12, _min12, _max12, value24, _min24, _max24, _x, _y)
    {
    }
};

class CParameter : public Parameter
{
public:
    CParameter(pchar title_ru,
        Value *value12, const Counter &_min12, const Counter &_max12,
        Value *value24, const Counter &_min24, const Counter &_max24,
        int _x = 0, int _y = 0) :
        Parameter(title_ru, value12, _min12, _max12, value24, _min24, _max24, _x, _y)
    {
    }
};

class Page : public Item
{
    friend struct Channel;

public:

    Page(Item **_items, void (*_additionalDraw)(), bool (*_func_start_test)()) :
        Item(),
        items(_items), additionalDraw(_additionalDraw), func_start_test(_func_start_test)
    {}

    virtual void Draw(int x, int y, int width, bool selected = false) override;

    virtual bool OnKey(Key::E) override;

    // Возвращает указатель на выделенный пункт меню
    Item *SelectedItem() { return items[selectedItem]; }

    // Возвращает true, если страница имеет дополнительную функцию отрисовки
    bool IsAddition() const { return additionalDraw != nullptr; }

    // Отображает значения параметров на изображении сигнала
    void DrawParameters() const;

    static const Page *ForCurrentSignal();

    // Запустить тест
    bool StartTest() const;

    // Возвращает true, если текущая страница - страница сигнала
    static bool IsSignal(Page *);

    // Возвращает количество итемов на странице
    int NumItems() const;

    // Отобразить состояние сервера
    static void DrawConnectionServer();

    bool ConsistOpenedItems() const;

    Parameter *GetCurrentParameter()
    {
        return current;
    }

protected:

    // Возвращает ширину элемента меню с номером num
    int WidthItem(int num) const;

    // Указатель на массив элементов меню. Заканчивается нулём.
    Item **items;

    void (*additionalDraw)();

    bool (*func_start_test)();

private:

    // Номер выбранного итема
    int selectedItem = 0;

    // "Текущий" параметр - тот, который сейчас выделен в меню. nullptr, если текущций элемент
    // меню - не параметр
    Parameter *current = nullptr;
};


class PageModes : public Page
{
public:
    PageModes(Item **_items) : Page(_items, nullptr, nullptr) {}
};
