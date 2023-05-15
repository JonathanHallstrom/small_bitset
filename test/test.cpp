#include "../src/small_bitset.hpp"
#include <array>
#include <bitset>
#include <limits>
#include <iostream>

static_assert(sizeof(small_bitset::small_bitset<8>) == 1);
static_assert(small_bitset::small_bitset<1>{1}[0]);
static_assert(small_bitset::small_bitset<2>{2}[1]);
static_assert(small_bitset::small_bitset<3>{4}[2]);
static_assert(~small_bitset::small_bitset<1>{}[0]);

static_assert((small_bitset::small_bitset<9>{256} >> 8)[0]);
static_assert(!(small_bitset::small_bitset<9>{256} >> 8)[8]);
static_assert((small_bitset::small_bitset<2>{2} >> 1)[0]);
static_assert((small_bitset::small_bitset<3>{4} >> 2)[0]);
static_assert((small_bitset::small_bitset<10>{512} >> 9)[0]);
static_assert((small_bitset::small_bitset<9>{256} >> 8) == small_bitset::small_bitset<9>{1});

static_assert((small_bitset::small_bitset<9>{1} << 8)[8]);
static_assert(!(small_bitset::small_bitset<9>{1} << 8)[0]);
static_assert((small_bitset::small_bitset<2>{1} << 1)[1]);

static_assert(small_bitset::small_bitset<1>(1).to_ullong() == 1);
static_assert(small_bitset::small_bitset<1>(2).to_ullong() == 2);
static_assert(small_bitset::small_bitset<64>(std::numeric_limits<unsigned long long>::max()).to_ullong() == std::numeric_limits<unsigned long long>::max());


int main() {
    small_bitset::small_bitset<8> s;

    s[0] = 1;
    s[1] = 1;

    std::cout << s[0] << '\n';
    std::cout << (small_bitset::small_bitset<10>{256}).to_string() << '\n';
    std::cout << (small_bitset::small_bitset<10>{256} >> 8).to_string() << '\n';
}
