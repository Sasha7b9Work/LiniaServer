// 2023/11/15 14:58:19 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "SCPI/Parser.h"


namespace Parser
{
    static pchar GetWord(pchar string, int pos_start, int pos_end, BufferParser *out);
}


pchar Parser::GetWord(pchar string, int num, BufferParser *out)
{
    out->data[0] = '\0';

    int pos_start = 0;
    int pos_end = 0;

    if (num == 1)
    {
        pos_start = -1;
    }
    else
    {
        const char *p = string;

        int current_word = 1;

        bool in_word = (*p != ' ') && (*p != ',') && (*p != ':') && (*p != '=');

        while (current_word < num && *p)
        {
            if (*p == ' ' || *p == ',' || *p == ':' || *p == '=')
            {
                if (in_word)
                {
                    in_word = false;
                }
            }
            else
            {
                if (!in_word)
                {
                    current_word++;
                    in_word = true;
                    if (current_word == num)
                    {
                        pos_start = p - string - 1;
                        break;
                    }
                }
            }

            p++;
        }

        if (current_word != num)
        {
            return "";
        }
    }

    pos_end = pos_start + 1;

    const char *p = string + pos_end;

    while (*p != '\0' && *p != ' ' && *p != ',' && *p != ':' && *p != '=')
    {
        pos_end++;
        p++;
    }

    return GetWord(string, pos_start, pos_end, out);
}


pchar Parser::GetWord(pchar string, int pos_start, int pos_end, BufferParser *out)
{
    if (pos_end - pos_start <= 0)
    {
        LOG_ERROR("Wrong arguments");

        out->data[0] = '\0';
    }
    else
    {
        char *p = out->data;

        for (int i = pos_start + 1; i < pos_end; i++)
        {
            *p++ = string[i];
        }

        *p = '\0';
    }

    return out->data;
}
