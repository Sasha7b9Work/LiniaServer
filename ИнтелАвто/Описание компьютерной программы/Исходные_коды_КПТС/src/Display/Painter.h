// 2025/02/03 15:05:41 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Display/Colors_.h"


namespace Painter
{
    void FillRegion(int x, int y, int width, int height, const Color &);

    int DrawText(int x, int y, const char *text, const Color & = Color(Color::COUNT));

    int DrawTextWithLimitationC(int x, int y, const char *text, const Color &, int limitX, int limitY, int limitWidth, int limitHeight);

    // Пишет строку в позиции x, y
    int DrawFormText(int x, int y, const Color &, pString text, ...);

    // Нарисовать горизонтальную линию
    void DrawHLine(int y, int x0, int x1, const Color & = Color(Color::COUNT));

    void DrawRectangle(int x, int y, int width, int height, const Color & = Color(Color::COUNT));
}
