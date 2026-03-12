// 2025/02/11 09:41:58 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Utils/Buffer_.h"
#include "Utils/String_.h"

/*
+   *IDN?
+   :SIGNAL <1, 2a, 2b, 3a, 3b, 4, 5a, 5b>
+   :SIGNAL?
+   :MODE <12V, 24>
+   :MODE?
                         1 2a 2b 3a 3b 4 5a 5b
    :PARAM:Us            + +     +  +  + +  +
    :PARAM:td                 +          +  +
    :PARAM:Ri                            +  +
    :PARAM:N             + +  +  +  +  + +  +
    :PARAM:Period        + +
    :PARAM:Imax          + +     +  +
    :PARAM:Ua                          +
    :PARAM:t7                          +
    :PARAM:t9                          +
*/


struct DirectionSCPI
{
    enum E
    {
        USB,
        LAN
    };
};


namespace SCPI
{
    struct Command;

    class InBuffer : public Buffer2048
    {
    public:
        InBuffer(DirectionSCPI::E _dir) : dir(_dir) { }
        void Update();
    private:
        Command *ParseCommand(pchar);
        String<> FirstWord(pchar);
        Command *ExtractCommand();
        DirectionSCPI::E dir;
    };

    void AppendNewData(DirectionSCPI::E, uint8 *, uint);

    void Update();
}
