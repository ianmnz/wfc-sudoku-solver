#pragma once

#include <random>
#include <vector>
#include <algorithm>


namespace randomns
{

inline std::random_device rd;
inline std::mt19937 g(rd());

// inline std::seed_seq seed{2};
// inline std::mt19937 g(seed);


std::vector<int> sample(const std::vector<int>& array, const int nb_elem = 1)
{
    std::vector<int> chosen;
    chosen.reserve(nb_elem);

    std::sample(array.cbegin(), array.cend(), std::back_inserter(chosen), nb_elem, g);

    return chosen;
}

void shuffle(std::vector<int>& array)
{
    std::shuffle(array.begin(), array.end(), g);
}

} // namespace randomns
