// 2022/01/12 10:19:53 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/HAL/HAL.h"
#include "Settings/Settings.h"
#include <stm32f4xx_hal.h>


#define CLEAR_FLASH_FLAGS \
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP    |  /* end of operation flag              */   \
                           FLASH_FLAG_OPERR  |  /* operation error flag               */   \
                           FLASH_FLAG_WRPERR |  /* write protected error flag         */   \
                           FLASH_FLAG_PGAERR |  /* programming alignment error flag   */   \
                           FLASH_FLAG_PGPERR |  /* programming parallelism error flag */   \
                           FLASH_FLAG_PGSERR);  /* programming sequence error flag    */


// Возвращает первый адрес, значение в котором равно 0xffffffff (можно записывать). Поиск начинается с адреса start,
// продолжается в участке памяти размером sizeFull. Кратно размеру sizeObject
static uint FindFirstFreeRecord(uint start, uint sizeSector, uint sizeRecord);

// Возвращает адрес последнего блока, в котором первый байт не равен 0xffffffff (в нём сохраенены последние настройки)
static uint FindLastOccupiedRecord(uint start, uint sizeSector, uint sizeRecord);

// Возвращает системный идентификатор сектора с начальным адресом address. Ежели такового нету, возвращает -1
static uint GetSector(uint address);


bool HAL_EEPROM::LoadSettings(Settings *settings)
{
    uint address = FindLastOccupiedRecord(ADDR_SECTOR_SETTINGS, SIZE_SECTOR_SETTINGS, sizeof(Settings));

    if (address)                                            // Если нашли сохранённую запись
    {
        uint size = *((uint *)address);

        if (size == sizeof(Settings))
        {
            *settings = *(reinterpret_cast<Settings *>(address));      // То запишем её в целевой объект

            return true;
        }
        else
        {
            EraseSector(ADDR_SECTOR_SETTINGS);

            return false;
        }
    }

    return false;
}


void HAL_EEPROM::SaveSettings(Settings *settings)
{
    uint address = FindFirstFreeRecord(ADDR_SECTOR_SETTINGS, SIZE_SECTOR_SETTINGS, sizeof(Settings));

    if (address == 0)
    {
        EraseSector(ADDR_SECTOR_SETTINGS);
        address = ADDR_SECTOR_SETTINGS;
    }

    settings->size = sizeof(Settings);

    WriteData(address, settings, sizeof(Settings));
}


static uint FindFirstFreeRecord(uint start, uint sizeFull, uint sizeRecord)
{
    uint address = start;
    uint end = start + sizeFull;

    while (address < end)
    {
        if (*(reinterpret_cast<uint *>(address)) == 0xffffffffU)
        {
            return address;
        }
        address += sizeRecord;
    }

    return 0;
}


static uint FindLastOccupiedRecord(uint start, uint sizeSector, uint sizeRecord)
{
    uint address = FindFirstFreeRecord(start, sizeSector, sizeRecord);

    if (address == 0)                               // Если свободной записи нет, значит, весь сектора заполнен
    {
        return start + sizeSector - sizeRecord;     // надо считвать последнюю запись
    }

    if (address == start)                           // Если первая свободная запись находится в начале сектора,
    {                                               // то сектор пуст - запись в него не производилась
        return 0;                                   // Возвращаем 0 как признак того, что записей нет
    }

    return address - sizeRecord;                    // Во всех остальных случаях возвращаем адрес записи, предыдущей
                                                    // по отношению к первой свободной
}


void HAL_EEPROM::EraseSector(uint startAddress)
{
    if (GetSector(startAddress) == static_cast<uint>(-1))
    {
        return;
    }

    CLEAR_FLASH_FLAGS

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef isFLASH;
    isFLASH.TypeErase = TYPEERASE_SECTORS;
    isFLASH.Sector = GetSector(startAddress);
    isFLASH.NbSectors = 1;
    isFLASH.VoltageRange = VOLTAGE_RANGE_3;

    uint error = 0;

    HAL_FLASHEx_Erase(&isFLASH, &error);

    HAL_FLASH_Lock();
}


static uint GetSector(uint address)
{
    struct StructSector
    {
        uint number;
        uint startAddress;
    };

    static const StructSector sectors[] =
    {
        {FLASH_SECTOR_7, HAL_EEPROM::ADDR_SECTOR_SETTINGS},
        {}
    };

    int i = 0;
    while (sectors[i].startAddress)
    {
        if (sectors[i].startAddress == address)
        {
            return sectors[i].number;
        }
        i++;
    }

    return static_cast<uint>(-1);
}


void HAL_EEPROM::WriteData(uint address, void *data, uint size)
{
    CLEAR_FLASH_FLAGS

    HAL_FLASH_Unlock();

    for (uint i = 0; i < size; i++)
    {
        HAL_FLASH_Program(TYPEPROGRAM_BYTE, address++, (static_cast<uint8 *>(data))[i]);
    }

    HAL_FLASH_Lock();
}
