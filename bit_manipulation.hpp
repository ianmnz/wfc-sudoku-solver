#pragma once

#include <map>

namespace bit {

inline int16_t set(const int16_t x, const int n)    { return x | (1 << n); }
inline int16_t clear(const int16_t x, const int n)  { return x & ~(1 << n); }
inline int16_t toggle(const int16_t x, const int n) { return x ^ (1 << n); }
inline bool check(const int16_t x, const int n) { return (x >> n) & 1; }
int count(int16_t x);

inline std::map<int16_t, int> mask = {
    { 0b1000000001, 1},
    { 0b1000000010, 2},
    { 0b1000000100, 3},
    { 0b1000001000, 4},
    { 0b1000010000, 5},
    { 0b1000100000, 6},
    { 0b1001000000, 7},
    { 0b1010000000, 8},
    { 0b1100000000, 9},
};

} // namespace bit
