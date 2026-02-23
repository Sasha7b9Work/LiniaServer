// 2022/11/23 12:38:18 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "SCPI/Commands.h"
#include "Menu/Pages/Pages.h"
#include "Settings/Settings.h"


namespace SCPI
{
    void Send(DirectionSCPI::E, pchar);

    void Error(DirectionSCPI::E, pchar);
}


bool SCPI::CommandWithParameters::Execute(DirectionSCPI::E)
{
    return false;
}


bool SCPI::CommandIDN::Execute(DirectionSCPI::E dir)
{
    String<> message("OAO MNIPI, KPTS, v1.0.1");
    Send(dir, message.c_str());
    return true;
}
