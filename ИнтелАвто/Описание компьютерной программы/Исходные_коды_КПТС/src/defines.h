// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "no-warnings.h"

#define VERSION_BUILD 411

#define DATE_BUILD "2026-02-23 09:05:31"

#include <stdint.h>

#ifdef WIN32
    #define __asm(x)
    #define WIN32_LEAN_AND_MEAN
    #define __attribute(x)
    #define __attribute__(x)
    #define __IO
    #define __STATIC_INLINE inline
#else
    #include <stm32f4xx_hal.h>
#endif


typedef unsigned short         uint16;
typedef signed short           int16;
typedef unsigned char          uint8;
typedef unsigned char          uchar;
typedef signed char            int8;
typedef signed long long int   int64;
typedef unsigned long long int uint64;
typedef unsigned int           uint;
typedef const char * const     pString;
typedef const char *           pchar;

typedef void(*pFuncVII)(int, int);


#define MAX_UINT 0xFFFFFFFF

union BitSet16
{
    BitSet16(uint8 b0, uint8 b1)
    {
        byte[0] = b0;
        byte[1] = b1;
    }
    uint16 half_word;
    uint8  byte[2];
};


#define _bitset(bits)                               \
  ((uint8)(                                         \
  (((uint8)((uint)bits / 01)        % 010) << 0) |  \
  (((uint8)((uint)bits / 010)       % 010) << 1) |  \
  (((uint8)((uint)bits / 0100)      % 010) << 2) |  \
  (((uint8)((uint)bits / 01000)     % 010) << 3) |  \
  (((uint8)((uint)bits / 010000)    % 010) << 4) |  \
  (((uint8)((uint)bits / 0100000)   % 010) << 5) |  \
  (((uint8)((uint)bits / 01000000)  % 010) << 6) |  \
  (((uint8)((uint)bits / 010000000) % 010) << 7)))


#define BIN_U8(bits) (_bitset(0##bits))

#define BIN_U16(bits1, bits0) (BIN_U8(bits1) << 8 | BIN_U8(bits0))

#define BIN_U32(bits3, bits2, bits1, bits0) (BIN_U8(bits3) << 24 | BIN_U8(bits2) << 16 | BIN_U8(bits1) << 8 | BIN_U8(bits0))

#define HEX_FROM_2(hex1, hex0) ((uint)(0x##hex1) << 16 | (uint)0x##hex0)

#define _SET_BIT_VALUE(value, numBit, bitValue) ((value) |= ((bitValue) << (numBit)))
#define _GET_BIT(value, numBit) (((value) >> (numBit)) & 1)
#define _CLEAR_BIT(value, bit) ((value) &= (~(1 << (bit))))
#define _SET_BIT(value, bit) ((value) |= (1 << (bit)))


#define STR_BACK        "Выйти"
#define STR_NUM_VERSION "Cherem V1.2"
#define MODEL_RU        "Частотомер электронно-счетный Ч3-96/2"
#define MODEL_EN        "Electronic counting frequency meter Ch3-96/2"
#define NAME_MODEL      (LANG_IS_RU ? MODEL_RU : MODEL_EN)


struct Coord
{
    Coord(int _x = 0, int _y = 0) : x(_x), y(_y) { }
    int x;
    int y;
};


#include "Utils/Log_.h"
#include "Utils/Debug_.h"
