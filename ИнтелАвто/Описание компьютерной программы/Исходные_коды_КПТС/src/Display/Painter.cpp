// 2025/02/03 17:41:11 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Display/Painter.h"


void Painter::FillRegion(int /*x*/, int /*y*/, int /*width*/, int /*height*/, const Color &)
{

}

int Painter::DrawText(int /*x*/, int /*y*/, const char * /*text*/, const Color &)
{
    return 0;
}

int Painter::DrawTextWithLimitationC(int /*x*/, int /*y*/, const char * /*text*/, const Color &, int /*limitX*/, int /*limitY*/, int /*limitWidth*/, int /*limitHeight*/)
{
    return 0;
}

// Пишет строку в позиции x, y
int Painter::DrawFormText(int /*x*/, int /*y*/, const Color &, pString /*text*/, ...)
{
    return 0;
}

// Нарисовать горизонтальную линию
void Painter::DrawHLine(int /*y*/, int /*x0*/, int /*x1*/, const Color &)
{

}


void Painter::DrawRectangle(int /*x*/, int /*y*/, int /*width*/, int /*height*/, const Color &)
{

}
