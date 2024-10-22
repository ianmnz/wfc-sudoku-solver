#pragma once

#include <map>

namespace bit {

inline int16_t set(const int16_t x, const int n)    { return x | (1 << n); }
inline int16_t clear(const int16_t x, const int n)  { return x & ~(1 << n); }
inline int16_t toggle(const int16_t x, const int n) { return x ^ (1 << n); }
inline bool check(const int16_t x, const int n) { return (x >> n) & 1; }
int count(int16_t x);

inline std::map<int16_t, int> mask = {
    { 0b00'0000'0001, 1}, { 0b10'0000'0001, 1},
    { 0b00'0000'0010, 2}, { 0b10'0000'0010, 2},
    { 0b00'0000'0100, 3}, { 0b10'0000'0100, 3},
    { 0b00'0000'1000, 4}, { 0b10'0000'1000, 4},
    { 0b00'0001'0000, 5}, { 0b10'0001'0000, 5},
    { 0b00'0010'0000, 6}, { 0b10'0010'0000, 6},
    { 0b00'0100'0000, 7}, { 0b10'0100'0000, 7},
    { 0b00'1000'0000, 8}, { 0b10'1000'0000, 8},
    { 0b01'0000'0000, 9}, { 0b11'0000'0000, 9},
};

} // namespace bit
