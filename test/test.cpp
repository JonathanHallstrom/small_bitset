#include "../src/small_bitset.hpp"
#include <array>
#include <bitset>
#include <iostream>
#include <limits>
#include <random>

static_assert(sizeof(sb::small_bitset<8>) == 1, "");
static_assert(sizeof(sb::small_bitset<16>) == 2, "");
static_assert(sizeof(sb::small_bitset<24>) == 3, "");
static_assert(sizeof(sb::small_bitset<64>) == 8, "");

static_assert(alignof(sb::small_bitset<8>) == 1, "");
static_assert(alignof(sb::small_bitset<56>) == 1, "");
static_assert(alignof(sb::small_bitset<57>) == alignof(std::size_t), "");
static_assert(alignof(sb::small_bitset<64>) == alignof(std::size_t), "");
static_assert(alignof(sb::small_bitset<1024>) == alignof(std::size_t), "");

static_assert(sb::small_bitset<1>{1}[0], "");
static_assert(sb::small_bitset<2>{2}[1], "");
static_assert(sb::small_bitset<3>{4}[2], "");
static_assert(~sb::small_bitset<1>{}[0], "");

static_assert((sb::small_bitset<9>{256} >> 8)[0], "");
static_assert(!(sb::small_bitset<9>{256} >> 8)[8], "");
static_assert((sb::small_bitset<2>{2} >> 1)[0], "");
static_assert((sb::small_bitset<3>{4} >> 2)[0], "");
static_assert((sb::small_bitset<10>{512} >> 9)[0], "");
static_assert((sb::small_bitset<9>{256} >> 8) == sb::small_bitset<9>{1}, "");
static_assert((sb::small_bitset<9>{1} << 8)[8], "");
static_assert(!(sb::small_bitset<9>{1} << 8)[0], "");
static_assert((sb::small_bitset<2>{1} << 1)[1], "");

static_assert(sb::small_bitset<1>(1).to_ulong() == 1, "");
static_assert(sb::small_bitset<1>(2).to_ulong() == 2, "");
static_assert(sb::small_bitset<8 * sizeof(unsigned long)>(std::numeric_limits<unsigned long>::max()).to_ulong() == std::numeric_limits<unsigned long>::max(), "");

static_assert(sb::small_bitset<1>(1).to_ullong() == 1, "");
static_assert(sb::small_bitset<1>(2).to_ullong() == 2, "");
static_assert(sb::small_bitset<8 * sizeof(unsigned long long)>(std::numeric_limits<unsigned long long>::max()).to_ullong() == std::numeric_limits<unsigned long long>::max(), "");


int main() {
    constexpr int size = 73; // 64 + 8 + 1
    sb::small_bitset<size> sb{};
    std::bitset<size> stdb{};

    std::mt19937_64 mt{std::random_device{}()};

    using udi = std::uniform_int_distribution<int>;

    for (int _ = 0; _ < (1 << 20); ++_) {
        switch (udi{0, 7}(mt)) {
            case 0: {
                int i = udi{0, sb.size() - 1}(mt);
                sb[i] = true;
                stdb[i] = true;
            } break;
            case 1: {
                int i = udi{0, sb.size() - 1}(mt);
                sb[i] = false;
                stdb[i] = false;
            } break;
            case 2: {
                sb.flip();
                stdb.flip();
            } break;
            case 3: {
                sb.reset();
                stdb.reset();
            } break;
            case 4: {
                sb.set();
                stdb.set();
            } break;
            case 5: {
                int i = udi{0, sb.size() - 1}(mt);
                sb[i] = ~sb[i];
                stdb[i] = ~stdb[i];
            } break;
            case 6: {
                int i = udi{0, sb.size() - 1}(mt);
                sb >>= i;
                stdb >>= i;
            } break;
            case 7: {
                int i = udi{0, sb.size() - 1}(mt);
                sb <<= i;
                stdb <<= i;
            } break;
            default: {
            } break;
        }
        assert(sb.all() == stdb.all());
        assert(sb.any() == stdb.any());
        assert(sb.none() == stdb.none());
        assert(sb.count() == stdb.count());
        for (std::size_t i = 0; i < sb.size(); ++i)
            assert(sb[i] == stdb[i]);
    }
    std::cout << "All tests passed!\n";
}
