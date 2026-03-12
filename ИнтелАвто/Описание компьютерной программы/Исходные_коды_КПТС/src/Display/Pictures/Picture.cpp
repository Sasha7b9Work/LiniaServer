// 2025/01/29 13:58:30 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Display/Pictures/Picture.h"
#include "Display/Pictures/Signal1.inc"
#include "Display/Pictures/Signal2a.inc"
#include "Display/Pictures/Signal2b.inc"
#include "Display/Pictures/Signal3a.inc"
#include "Display/Pictures/Signal3b.inc"
#include "Display/Pictures/Signal4.inc"
#include "Display/Pictures/Signal5a.inc"
#include "Display/Pictures/Signal5b.inc"
#include "Display/Primitives_.h"
#include "Hardware/Timer.h"
#include "Display/Display_.h"
#include <miniz/miniz.h>
#include <cstring>


using namespace Primitives;


namespace Picture
{
    static const unsigned char *archives[TypePicture::_Count] =
    {
        bmp_zip_Signal1,
        bmp_zip_Signal2a,
        bmp_zip_Signal2b,
        bmp_zip_Signal3a,
        bmp_zip_Signal3b,
        bmp_zip_Signal4,
        bmp_zip_Signal5a,
        bmp_zip_Signal5b,
        nullptr,
        nullptr
    };

    // \warn сюда нельзя распаковать картинку больше 64 кБ
    static uint8 buffer[1024 * 64] __attribute__ ((section("CCM_DATA")));
    static TypePicture::E prev_type = TypePicture::_Count;

    static bool Uncompress(TypePicture::E);

    static unsigned long CalculateSize(TypePicture::E);

    static void DrawScheme(TypePicture::E);

    static void DrawHorLine(int x, int y, int width);
    static void DrawVerLine(int x, int y, int height);

    // dir:
    // 0 - вправо, 1 - вниз, 2 - влево, 3 - вверх
    static void DrawArrow(int x, int y, int dir);
}


bool Picture::Uncompress(TypePicture::E type)
{
    if (type == prev_type)
    {
        return true;
    }

    prev_type = TypePicture::_Count;

    mz_zip_archive zip_archive;
    std::memset(&zip_archive, 0, sizeof(zip_archive));

    if (mz_zip_reader_init_mem(&zip_archive, archives[type], CalculateSize(type), 0))
    {
        mz_zip_archive_file_stat file_stat;

        if (mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
        {
            if (mz_zip_reader_extract_file_to_mem(&zip_archive, file_stat.m_filename, buffer, (size_t)file_stat.m_uncomp_size, 0))
            {
                prev_type = type;
            }
        }
    }

    mz_zip_reader_end(&zip_archive);

    return (prev_type != TypePicture::_Count);
}


void Picture::DrawPicure(int x, int y, TypePicture::E type)
{
#pragma pack(push)
#pragma pack(1)

    struct StructureBMP
    {
        uint8   magic[2];

        uint    file_size;
        uint16  creator1;
        uint16  creator2;
        uint    bmp_offset;

        uint    header_size;
        int     width;
        int     height;
        uint16  num_planes;
        uint16  bits_per_pixel;
        uint    compression;
        uint    bmp_byte_size;
        int     hres;
        int     vres;
        uint    num_colors;
        uint    num_important_colors;
    };

    if (type >= TypePicture::Scheme1)
    {
        DrawScheme(type);
    }
    else
    {
        if (Uncompress(type))
        {
            StructureBMP *head = (StructureBMP *)buffer; //-V641

            uint *colors = (uint *)((uint8 *)buffer + 14 + head->header_size);          // Находим таблицу цветов

            uint8 *pixel = ((uint8 *)buffer) + head->bmp_offset;

            for (int j = y + head->height; j > y; j--)
            {
                for (int i = x; i < x + head->width; i++)
                {
                    uint color = colors[*pixel++];                                      // Находим цвет

                    if ((color & 0xFFFFFF00) != 0xFFFFFF00)                             // И ставим точку, если цвет - не белый
                    {
                        Point().Draw(i, j, Color::WHITE);
                    }
                }
            }
        }
    }

#pragma pack(pop)
}


unsigned long Picture::CalculateSize(TypePicture::E type)
{
    static unsigned long sizes[TypePicture::_Count] =
    {
        sizeof(bmp_zip_Signal1),
        sizeof(bmp_zip_Signal2a),
        sizeof(bmp_zip_Signal2b),
        sizeof(bmp_zip_Signal3a),
        sizeof(bmp_zip_Signal3b),
        sizeof(bmp_zip_Signal4),
        sizeof(bmp_zip_Signal5a),
        sizeof(bmp_zip_Signal5b),
        0,
        0
    };

    return sizes[type];
}


void Picture::DrawScheme(TypePicture::E type)
{
    Color::FILL.SetAsCurrent();

    const int width = 120;
    const int height = 60;

    Rect rect(width, height);

    int x0 = 160;
    int y0 = 50;

    int x1 = x0;
    int x2 = x0 + 180;
    int y1 = y0;
    int y2 = y0 + 100;

    rect.Draw(x1, y1, 2);
    rect.Draw(x2, y1, 2);
    rect.Draw(x2, y2, 2);

    int dx = 10;
    int dy = 20;

    {
        int xx1 = x2 + width * 2 / 3;
        int yy1 = y1 + dy;
        int yy2 = y1 + height - dy;

        int ddx = yy2 - yy1;

        yy1 = y1 + height - dx;
        yy2 = y2 + dx;

        DrawVerLine(xx1, yy1, yy2 - yy1);
        DrawArrow(xx1, (yy1 + yy2) / 2, 1);
        DrawVerLine(xx1 + ddx, yy1, yy2 - yy1);
        DrawArrow(xx1 + ddx, (yy1 + yy2) / 2, 1);
    }

    TypeFont::E font = Font::Get();

    Font::Set(TypeFont::GOSTAU16BOLD);

    if (type == TypePicture::Scheme1)
    {
        Circle circle(3);

        int xx1 = x1 + width - dx;
        int xx2 = x2 + dx;
        int yy1 = y1 + dy;
        int yy2 = y1 + height - dy;

        DrawHorLine(xx1, yy1, xx2 - xx1);
        DrawArrow((xx1 + xx2) / 2, yy1, 0);
        DrawHorLine(xx1, yy2, xx2 - xx1);
        DrawArrow((xx1 + xx2) / 2, yy2, 0);

        xx1 = x1 + width / 2;
        yy1 = (y1 + height + y2) / 2;
        xx2 = x2 + width / 3;

        HLine(xx2 - xx1).Draw(xx1, yy1);
        VLine(yy1 - y0 - height).Draw(xx1, y0 + height);
        VLine(yy1 - y0 - height).Draw(xx2, y0 + height);

        DrawArrow(xx1, y0 + height, 3);
        DrawArrow(xx2, y0 + height, 3);

        Color::GRAY_75.SetAsCurrent();

        Text("IT6523").WriteInCenterRect(x1, y1, width, height);
        Text("КПТС").WriteInCenterRect(x2, y1, width, height);
        Text("DUT").WriteInCenterRect(x2, y2, width, height);

        Text("LAN").Write(x1 + 135, y1 + height + 4);
    }
    else if (type == TypePicture::Scheme2)
    {
        int xx1 = x1 + width;
        int yy1 = y0 + height / 2;

        HLine(x2 - xx1).Draw(xx1, yy1);
        DrawArrow(xx1, yy1, 2);
        DrawArrow(x2, yy1, 0);

        Color::GRAY_75.SetAsCurrent();

        Text("КПТС").WriteInCenterRect(x1, y1, width, height);
        Text("IT6523").WriteInCenterRect(x2, y1, width, height);
        Text("DUT").WriteInCenterRect(x2, y2, width, height);

        Text("LAN").Write(xx1 + 17, yy1 - 16);
    }

    Font::Set(font);
}


void Picture::DrawHorLine(int x, int y, int width)
{
    Circle circle(3);

    circle.Fill(x, y, Color::FILL);
    circle.Fill(x + width, y, Color::FILL);
    HLine(width).Draw(x, y);
}


void Picture::DrawVerLine(int x, int y, int height)
{
    Circle circle(3);

    circle.Fill(x, y, Color::FILL);
    circle.Fill(x, y + height, Color::FILL);
    VLine(height).Draw(x, y);
}


void Picture::DrawArrow(int x, int y, int dir)
{
    static const int dl = 5;

    if (dir == 0)                                   // право
    {
        Line().Draw(x - dl, y - dl, x, y);
        Line().Draw(x - dl, y + dl, x, y);
    }
    else if (dir == 1)                              // вниз
    {
        Line().Draw(x - dl, y - dl, x, y);
        Line().Draw(x + dl, y - dl, x, y);
    }
    else if (dir == 2)                              // влево
    {
        Line().Draw(x, y, x + dl, y - dl);
        Line().Draw(x, y, x + dl, y + dl);
    }
    else if (dir == 3)                              // вверх
    {
        Line().Draw(x - dl, y + dl, x, y);
        Line().Draw(x + dl, y + dl, x, y);
    }
}
