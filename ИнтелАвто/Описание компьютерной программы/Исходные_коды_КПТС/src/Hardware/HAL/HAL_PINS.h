// 2024/02/08 21:13:39 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once


struct Pin
{
    Pin(void *_port, uint16 _pin) : port(_port), pin(_pin) { }
protected:
    void *port;
    uint16 pin;
};


struct PinOut : public Pin
{
    PinOut(void *_port, uint16 _pin) : Pin(_port, _pin) { }
    void Init();
    void ToHi();
    void ToLow();
};


struct PinIn : public Pin
{
    PinIn(void *_port, uint16 _pin) : Pin(_port, _pin) { }
    void Init();
    bool IsHi();
};
