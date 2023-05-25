#include "../src/small_bitset.hpp"
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <future>
#include <iostream>
#include <limits>
#include <random>
#include <thread>

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

template<int size>
void test() {
    std::mt19937_64 mt{std::random_device{}()};

    using udi = std::uniform_int_distribution<int>;

    sb::small_bitset<size> small{};
    std::bitset<size> standard{};

    for (int _ = 0; _ < (1 << 20); ++_) {
        switch (udi{0, 10}(mt)) {
            case 0: {
                int i = udi{0, small.size() - 1}(mt);
                small[i] = true;
                standard[i] = true;
            } break;
            case 1: {
                int i = udi{0, small.size() - 1}(mt);
                small[i] = false;
                standard[i] = false;
            } break;
            case 2: {
                small.flip();
                standard.flip();
            } break;
            case 3: {
                small.reset();
                standard.reset();
            } break;
            case 4: {
                small.set();
                standard.set();
            } break;
            case 5: {
                int i = udi{0, small.size() - 1}(mt);
                small[i] = ~small[i];
                standard[i] = ~standard[i];
            } break;
            case 6: {
                int i = udi{0, small.size() - 1}(mt);
                small >>= i;
                standard >>= i;
            } break;
            case 7: {
                int i = udi{0, small.size() - 1}(mt);
                small <<= i;
                standard <<= i;
            } break;
            case 8: {
                int i = udi{0, small.size() - 1}(mt);
                small |= small >> i;
                standard |= standard >> i;
            } break;
            case 9: {
                int i = udi{0, small.size() - 1}(mt);
                small ^= small << i;
                standard ^= standard << i;
            } break;
            case 10: {
                int i = udi{0, small.size() - 1}(mt);
                small &= small >> i;
                standard &= standard >> i;
            } break;
            default: {
            } break;
        }

        // std::cerr << "testing: " << small.to_string() << ' ' << standard.to_string() << '\n';
        assert(small.to_string() == standard.to_string());

        assert(small.all() == standard.all());
        assert(small.any() == standard.any());
        assert(small.none() == standard.none());
        assert(small.count() == standard.count());
        for (std::size_t i = 0; i < small.size(); ++i)
            assert(small[i] == standard[i]);
    }
}

int main() {
    std::vector<std::future<void>> futures;
#define LAUNCH(x) futures.push_back(std::async(std::launch::async, [&]() { x; }));
// #define LAUNCH(x) futures.push_back(std::async(std::launch::deferred, [&]() { x; }));
    LAUNCH(test<1>());
    LAUNCH(test<2>());
    LAUNCH(test<3>());
    LAUNCH(test<4>());
    LAUNCH(test<5>());
    LAUNCH(test<6>());
    LAUNCH(test<7>());
    LAUNCH(test<8>());
    LAUNCH(test<9>());
    LAUNCH(test<10>());
    LAUNCH(test<11>());
    LAUNCH(test<12>());
    LAUNCH(test<13>());
    LAUNCH(test<14>());
    LAUNCH(test<15>());
    LAUNCH(test<16>());
    LAUNCH(test<17>());
    LAUNCH(test<18>());
    LAUNCH(test<19>());
    LAUNCH(test<20>());
    LAUNCH(test<21>());
    LAUNCH(test<22>());
    LAUNCH(test<23>());
    LAUNCH(test<24>());
    LAUNCH(test<25>());
    LAUNCH(test<26>());
    LAUNCH(test<27>());
    LAUNCH(test<28>());
    LAUNCH(test<29>());
    LAUNCH(test<30>());
    LAUNCH(test<31>());
    LAUNCH(test<32>());
    LAUNCH(test<33>());
    LAUNCH(test<34>());
    LAUNCH(test<35>());
    LAUNCH(test<36>());
    LAUNCH(test<37>());
    LAUNCH(test<38>());
    LAUNCH(test<39>());
    LAUNCH(test<40>());
    LAUNCH(test<41>());
    LAUNCH(test<42>());
    LAUNCH(test<43>());
    LAUNCH(test<44>());
    LAUNCH(test<45>());
    LAUNCH(test<46>());
    LAUNCH(test<47>());
    LAUNCH(test<48>());
    LAUNCH(test<49>());
    LAUNCH(test<50>());
    LAUNCH(test<51>());
    LAUNCH(test<52>());
    LAUNCH(test<53>());
    LAUNCH(test<54>());
    LAUNCH(test<55>());
    LAUNCH(test<56>());
    LAUNCH(test<57>());
    LAUNCH(test<58>());
    LAUNCH(test<59>());
    LAUNCH(test<60>());
    LAUNCH(test<61>());
    LAUNCH(test<62>());
    LAUNCH(test<63>());
    LAUNCH(test<64>());
    LAUNCH(test<65>());
    LAUNCH(test<66>());
    LAUNCH(test<67>());
    LAUNCH(test<68>());
    LAUNCH(test<69>());
    LAUNCH(test<70>());
    LAUNCH(test<71>());
    LAUNCH(test<72>());
    LAUNCH(test<73>());
    LAUNCH(test<74>());
    LAUNCH(test<75>());
    LAUNCH(test<76>());
    LAUNCH(test<77>());
    LAUNCH(test<78>());
    LAUNCH(test<79>());
    LAUNCH(test<80>());
    LAUNCH(test<81>());
    LAUNCH(test<82>());
    LAUNCH(test<83>());
    LAUNCH(test<84>());
    LAUNCH(test<85>());
    LAUNCH(test<86>());
    LAUNCH(test<87>());
    LAUNCH(test<88>());
    LAUNCH(test<89>());
    LAUNCH(test<90>());
    LAUNCH(test<91>());
    LAUNCH(test<92>());
    LAUNCH(test<93>());
    LAUNCH(test<94>());
    LAUNCH(test<95>());
    LAUNCH(test<96>());
    LAUNCH(test<97>());
    LAUNCH(test<98>());
    LAUNCH(test<99>());
    LAUNCH(test<100>());
    LAUNCH(test<101>());
    LAUNCH(test<102>());
    LAUNCH(test<103>());
    LAUNCH(test<104>());
    LAUNCH(test<105>());
    LAUNCH(test<106>());
    LAUNCH(test<107>());
    LAUNCH(test<108>());
    LAUNCH(test<109>());
    LAUNCH(test<110>());
    LAUNCH(test<111>());
    LAUNCH(test<112>());
    LAUNCH(test<113>());
    LAUNCH(test<114>());
    LAUNCH(test<115>());
    LAUNCH(test<116>());
    LAUNCH(test<117>());
    LAUNCH(test<118>());
    LAUNCH(test<119>());
    LAUNCH(test<120>());
    LAUNCH(test<121>());
    LAUNCH(test<122>());
    LAUNCH(test<123>());
    LAUNCH(test<124>());
    LAUNCH(test<125>());
    LAUNCH(test<126>());
    LAUNCH(test<127>());
    LAUNCH(test<128>());
    int done_count = 0;
    for (auto &&f: futures) {
        f.get();
        std::cerr << "Size " << ++done_count << " passed!\n";
    }
    std::cout << "All tests passed!\n";
}

// time g++ test/test.cpp -Wall -Wextra -Wpedantic -fsanitize=address,undefined -Og -g3