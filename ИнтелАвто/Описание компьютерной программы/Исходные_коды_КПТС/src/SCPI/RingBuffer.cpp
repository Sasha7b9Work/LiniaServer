// 2023/10/16 10:59:36 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "SCPI/RingBuffer.h"


void RingBuffer::Clear()
{
    index_in = 0;
    index_out = 0;
    num_elements = 0;
}

void RingBuffer::Append(uint8 byte)
{
    if (GetElementCount() == SIZE)
    {
        return;
    }

    buffer[index_in] = byte;
    index_in++;

    if (index_in == SIZE)
    {
        index_in = 0;
    }

    num_elements++;
}


int RingBuffer::GetElementCount() const
{
    int index_new = index_in;

    if (index_new >= index_out)
    {
        return index_new - index_out;
    }

    return SIZE + index_new - index_out;
}


uint8 RingBuffer::Pop()
{
    if (GetElementCount() == 0)
    {
        return 0;
    }

    num_elements--;

    uint8 result = buffer[index_out++];

    if (index_out == SIZE)
    {
        index_out = 0;
    }

    return result;
}


void RingBuffer::GetData(SCPI::InBuffer &out)
{
    while (GetElementCount() != 0)
    {
        out.Append(Pop());
    }
}
