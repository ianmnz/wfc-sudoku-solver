#include "bit_manipulation.hpp"

namespace bit
{

int count(int16_t x)
{
    int count = 0;

    do {
        count += (x & 1);
    } while (x >>= 1);

    return count;
}

} // namespace bit
