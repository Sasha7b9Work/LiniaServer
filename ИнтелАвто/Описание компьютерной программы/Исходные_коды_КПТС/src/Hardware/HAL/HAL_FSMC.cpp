// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/HAL/HAL.h"
#include "Display/Colors_.h"
#include <stm32f4xx_hal.h>


#define PIN_BL_E  GPIO_PIN_1
#define PORT_BL_E GPIOD
#define BL_E      PORT_BL_E, PIN_BL_E

#define PIN_D_C  GPIO_PIN_3
#define PORT_D_C GPIOD
#define D_C      PORT_D_C, PIN_D_C

#define PIN_WR  GPIO_PIN_7
#define PORT_WR GPIOD
#define WR      PORT_WR, PIN_WR

#define PIN_RD  GPIO_PIN_6
#define PORT_RD GPIOD
#define RD      PORT_RD, PIN_RD

#define PIN_CS  GPIO_PIN_5
#define PORT_CS GPIOD
#define CS      PORT_CS, PIN_CS

#define PIN_DIP_ON  GPIO_PIN_5
#define PORT_DIP_ON GPIOD
#define DIP_ON      PORT_DIP_ON, PIN_DIP_ON

#define PIN_RESET   GPIO_PIN_4
#define PORT_RESET  GPIOD
#define RESET       PORT_RESET, PIN_RESET

#define PORT_DATA GPIOE

struct StructPIO
{
    StructPIO(GPIO_TypeDef *g, uint16 p) : gpio(g), pin(p) { }

    void Set() const
    {
        HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
    }

    void Reset() const
    {
        HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
    }

    void Write(int state) const
    {
        HAL_GPIO_WritePin(gpio, pin, (GPIO_PinState)state);
    }

    int Read() const
    {
        return HAL_GPIO_ReadPin(gpio, pin);
    }

private:
    GPIO_TypeDef *gpio;
    uint16 pin;
};


StructPIO pinBL_E(BL_E);
StructPIO pinD_C(D_C);
StructPIO pinWR(WR);
StructPIO pinRD(RD);
StructPIO pinCS(CS);
StructPIO pinDIP_ON(DIP_ON);
StructPIO pinRESET(RESET);

StructPIO pinD0 (PORT_DATA, GPIO_PIN_0);
StructPIO pinD1 (PORT_DATA, GPIO_PIN_1);
StructPIO pinD2 (PORT_DATA, GPIO_PIN_2);
StructPIO pinD3 (PORT_DATA, GPIO_PIN_3);
StructPIO pinD4 (PORT_DATA, GPIO_PIN_4);
StructPIO pinD5 (PORT_DATA, GPIO_PIN_5);
StructPIO pinD6 (PORT_DATA, GPIO_PIN_6);
StructPIO pinD7 (PORT_DATA, GPIO_PIN_7);
StructPIO pinD8 (PORT_DATA, GPIO_PIN_8);
StructPIO pinD9 (PORT_DATA, GPIO_PIN_9);
StructPIO pinD10(PORT_DATA, GPIO_PIN_10);
StructPIO pinD11(PORT_DATA, GPIO_PIN_11);
StructPIO pinD12(PORT_DATA, GPIO_PIN_12);
StructPIO pinD13(PORT_DATA, GPIO_PIN_13);
StructPIO pinD14(PORT_DATA, GPIO_PIN_14);
StructPIO pinD15(PORT_DATA, GPIO_PIN_15);


struct DataBus
{
public:
    static void Init();
    // Установить данные на шину
    static void Set(uint16 data);
    // Читать данные с шины
    static uint16 Read();

    static void InitWrite();

public:

    static void InitRead();

    static bool forWrite;
};


bool DataBus::forWrite = true;


void HAL_BUS_DISPLAY::Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    DataBus::Init();

    GPIO_InitTypeDef is =
    {
        0,
        GPIO_MODE_OUTPUT_PP,
        GPIO_PULLUP,
        0,
        0
    };

    is.Pin = PIN_CS; //-V760
    HAL_GPIO_Init(PORT_CS, &is);

    is.Pin = PIN_DIP_ON;
    HAL_GPIO_Init(PORT_DIP_ON, &is);

    is.Pin = PIN_RESET;
    HAL_GPIO_Init(PORT_RESET, &is);

    is.Pin = PIN_BL_E;
    HAL_GPIO_Init(PORT_BL_E, &is);

    is.Pin = PIN_D_C;
    HAL_GPIO_Init(PORT_D_C, &is);

    is.Pin = PIN_WR;
    HAL_GPIO_Init(PORT_WR, &is);

    is.Pin = PIN_RD;
    HAL_GPIO_Init(PORT_RD, &is);

    pinRESET.Set();
    pinCS.Set();
    pinRD.Set();
    pinWR.Set();
    pinDIP_ON.Set();
    pinBL_E.Set();
}


void HAL_BUS_DISPLAY::Reset()
{
    pinRESET.Set();
    HAL_Delay(500);
    pinRESET.Reset();
    HAL_Delay(500);
    pinRESET.Set();
    HAL_Delay(500);
}


void HAL_BUS_DISPLAY::WriteCommand(uint16 command)
{
    pinCS.Reset();
    pinD_C.Reset();
    pinWR.Reset();

    DataBus::Set(command);

    pinWR.Set();
    pinD_C.Set();
    pinCS.Set();
}


void HAL_BUS_DISPLAY::WriteCommand(uint16 command, uint data)
{
    WriteCommand(command);
    WriteData(data);
}


void HAL_BUS_DISPLAY::WriteCommand(uint16 command, uint data1, uint data2, uint data3)
{
    WriteCommand(command);
    WriteData(data1);
    WriteData(data2);
    WriteData(data3);
}


void HAL_BUS_DISPLAY::WriteData(uint data)
{
    //pinCS.Reset();
    PORT_CS->BSRR = PIN_CS << 16;

    //pinD_C.Set();
    PORT_D_C->BSRR = PIN_D_C;

    //pinWR.Reset();
    PORT_WR->BSRR = PIN_WR << 16;

    DataBus::Set((uint16)data);

    //pinWR.Set();
    PORT_WR->BSRR = PIN_WR;

    //pinCS.Set();
    PORT_CS->BSRR = PIN_CS;
}


static void WindowSet(int s_x, int e_x, int s_y, int e_y)
{
    HAL_BUS_DISPLAY::WriteCommand(0x2a);               //SET page address

    HAL_BUS_DISPLAY::WriteData((uint)((s_x) >> 8));    //SET start page address=0
    HAL_BUS_DISPLAY::WriteData((uint)s_x);
    HAL_BUS_DISPLAY::WriteData((uint)(e_x) >> 8);      //SET end page address
    HAL_BUS_DISPLAY::WriteData((uint)e_x);

    HAL_BUS_DISPLAY::WriteCommand(0x2b);               //SET column address

    HAL_BUS_DISPLAY::WriteData((uint)(s_y) >> 8);      //SET start column address=0
    HAL_BUS_DISPLAY::WriteData((uint)s_y);
    HAL_BUS_DISPLAY::WriteData((uint)(e_y) >> 8);      //SET end column address
    HAL_BUS_DISPLAY::WriteData((uint)e_y);
}


#ifdef WIN32
    #define DELAY
#else
    #define DELAY  __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); \
                   __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); \
                   __NOP(); __NOP(); __NOP(); __NOP(); __NOP()
#endif


void HAL_BUS_DISPLAY::SendBuffer(uint8 *buffer, int x, int y, int width, int height, int k)
{
    int top = x;
    int right = x + width - 1;

    int left = y;
    int bottom = y + height - 1;

    WindowSet(top, right, left, bottom);

    HAL_BUS_DISPLAY::WriteCommand(0x2c);   // Write memory start

    DataBus::InitWrite();

    PORT_CS->BSRR = PIN_CS << 16;

    PORT_D_C->BSRR = PIN_D_C;

    uint *colors = ColorScheme::Current().colors;

    uint color1 = colors[*buffer++];
    uint color2 = colors[*buffer++];

    int count = width * height / 2 / k;

    for(int i = 0; i < count; i++)
    {
        PORT_WR->BSRR = PIN_WR << 16;
        uint16 col3 = (uint16)color2;
        uint16 value = (uint16)(color1 << 8);
        DELAY;
        PORT_DATA->ODR = (uint)(value | (uint16)(color1 >> 8));
        PORT_WR->BSRR = PIN_WR;
        DELAY;

        value |= (uint8)(color2 >> 16);
        DELAY;
        PORT_WR->BSRR = PIN_WR << 16; //-V779
        DELAY;
        color1 = colors[*buffer++];
        PORT_DATA->ODR = value;
        DELAY;
        PORT_WR->BSRR = PIN_WR;
        DELAY;
        color2 = colors[*buffer++];
        DELAY;
        PORT_WR->BSRR = PIN_WR << 16;
        DELAY;
        PORT_DATA->ODR = col3;
        PORT_WR->BSRR = PIN_WR;
        DELAY;
    }

    PORT_CS->BSRR = PIN_CS;
}


uint16 HAL_BUS_DISPLAY::ReadData()
{
    pinCS.Reset();
    pinD_C.Set();
    pinRD.Reset();

    uint16 result = DataBus::Read();

    pinRD.Set();
    pinCS.Set();

    return result;
}


uint16 HAL_BUS_DISPLAY::GetData(uint16 address)
{
    WriteCommand(address);
    return ReadData();
}


void DataBus::Init()
{
    InitWrite();
}


void DataBus::Set(uint16 data)
{
    if(!forWrite)
    {
        InitWrite();
    }

    PORT_DATA->ODR = data;
}


uint16 DataBus::Read()
{
    if(forWrite)
    {
        InitRead();
    }

    static const StructPIO pins[16] =
    {
        pinD0, pinD1, pinD2, pinD3, pinD4, pinD5, pinD6, pinD7,
        pinD8, pinD9, pinD10, pinD11, pinD12, pinD13, pinD14, pinD15
    };

    uint16 result = 0;

    for(int i = 0; i < 16; i++)
    {
        uint16 bit = (uint16)((pins[i].Read() & 0x01) << i);

        result |= bit;
    }

    return result;
}



void DataBus::InitWrite()
{
    GPIO_InitTypeDef is =
    {
        0,
        GPIO_MODE_OUTPUT_PP,
        GPIO_PULLUP,
        0,
        0
    };

    is.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(PORT_DATA, &is);

    forWrite = true;
};


void DataBus::InitRead()
{
    GPIO_InitTypeDef is =
    {
        0,
        GPIO_MODE_INPUT,
        GPIO_PULLDOWN,
        0,
        0
    };

    is.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(PORT_DATA, &is);

    forWrite = false;
};
