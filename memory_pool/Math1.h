#ifndef MATH1_H_
#define MATH1_H_
#include<cstddef>

namespace math1
{
    std::size_t getGcd(std::size_t a, std::size_t b)
    {
        while(b > 0)
        {
            std::size_t temp = b % a;
            a = b;
            b = temp;
        }
        return a;
    }

    std::size_t getLcm(std::size_t value1, std::size_t value2)
    {
        std::size_t gcd = getGcd(value1, value2);
        std::size_t result = value1 * value2 / gcd;
        return result;
    }
}
#endif