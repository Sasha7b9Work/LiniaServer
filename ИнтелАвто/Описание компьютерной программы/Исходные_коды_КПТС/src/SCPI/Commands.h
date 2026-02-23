// 2022/11/23 11:21:58 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Utils/String_.h"
#include "SCPI/SCPI.h"


namespace SCPI
{
    struct Command
    {
        virtual bool Execute(DirectionSCPI::E) = 0;
        virtual ~Command() { }
    };


    struct CommandNull : public Command
    {
        virtual bool Execute(DirectionSCPI::E) override { return false; }
    };


    struct CommandWithParameters : public Command
    {
        CommandWithParameters(pchar _params) { params.SetFormat(_params); }
        virtual bool Execute(DirectionSCPI::E) override;
    protected:
        String<> params;
    };


    struct CommandIDN : public Command
    {
        virtual bool Execute(DirectionSCPI::E) override;
    };
}
