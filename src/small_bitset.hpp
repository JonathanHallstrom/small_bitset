#ifndef SMALL_BITSET_H
#define SMALL_BITSET_H

#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>

namespace sb {
static constexpr std::uint8_t masks[] = {1, 2, 4, 8, 16, 32, 64, 128};
template<std::size_t num_bits>
class small_bitset {

private:
    class bit_ref {
        friend small_bitset;

    private:
        std::uint8_t *byte;
        std::size_t which{};

    private:
        constexpr bit_ref(std::uint8_t &byte, std::size_t which) : byte{&byte}, which{which} {}

    public:
        constexpr bit_ref operator=(bool b) {
            if (b) {
                *byte |= masks[which];
            } else {
                *byte &= ~masks[which];
            }
            return *this;
        }

        constexpr bool operator~() const {
            return (*byte & masks[which]) == 0;
        }

        constexpr operator bool() const {
            return (*byte & masks[which]) != 0;
        }
    };

public:
    static constexpr std::size_t num_bytes = num_bits / 8 + (num_bits % 8 != 0);

    static_assert(num_bits > 0 && "number of bits has to be greater than zero");

    struct big_version {
        union {
            std::size_t register_size_arr[num_bytes / sizeof(std::size_t)];
            std::uint8_t byte_size_arr[num_bytes]{};
        } data;

        constexpr std::uint8_t &operator[](std::size_t idx) {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t const &operator[](std::size_t idx) const {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t *begin() {
            return data.byte_size_arr;
        }

        constexpr std::uint8_t const *begin() const {
            return data.byte_size_arr;
        }

        constexpr std::uint8_t *end() {
            return data.byte_size_arr + num_bytes;
        }

        constexpr std::uint8_t const *end() const {
            return data.byte_size_arr + num_bytes;
        }
    };

    struct small_version {
        struct empty {
            template<class... G> constexpr empty(G &&...) noexcept { throw "unreachable"; }
            template<class G> constexpr empty &operator=(G &&) const noexcept { throw "unreachable"; }
            template<class G> constexpr operator G() const noexcept { throw "unreachable"; }
        };
        union {
            empty register_size_arr[0]; // never accessed
            std::uint8_t byte_size_arr[num_bytes]{};
        } data;

        constexpr std::uint8_t &operator[](std::size_t idx) {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t const &operator[](std::size_t idx) const {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t *begin() {
            return data.byte_size_arr;
        }

        constexpr std::uint8_t const *begin() const {
            return data.byte_size_arr;
        }

        constexpr std::uint8_t *end() {
            return data.byte_size_arr + num_bytes;
        }

        constexpr std::uint8_t const *end() const {
            return data.byte_size_arr + num_bytes;
        }
    };

    typename std::conditional<(num_bytes >= 8), big_version, small_version>::type data{};

    constexpr small_bitset() = default;

    constexpr small_bitset(small_bitset const &other) = default;

    constexpr small_bitset(std::uint64_t u) {
        std::size_t i = 0;
        while (u) {
            assert(i < num_bytes && "u does not fit in bitset");
            data[i++] = u & 0xff;
            u >>= 8;
        }
    }

    constexpr bool operator==(small_bitset other) const {
        for (std::size_t i = 0; i < num_bytes; ++i)
            if (data[i] != other.data[i])
                return false;
        return true;
    }

    constexpr bool operator!=(small_bitset other) const {
        return !(*this == other);
    }

    constexpr bit_ref operator[](std::size_t idx) {
        return bit_ref{data[idx / 8], idx % 8};
    }

    constexpr bool operator[](std::size_t idx) const {
        return (data[idx / 8] & masks[idx % 8]) != 0;
    }

    constexpr bool test(std::size_t idx) const {
        return (data[idx / 8] & masks[idx % 8]) != 0;
    }

    constexpr void set(std::size_t idx) {
        data[idx / 8] |= masks[idx % 8];
    }

    constexpr void set(std::size_t idx, bool value) {
        if (value) {
            data[idx / 8] |= masks[idx % 8];
        } else {
            data[idx / 8] &= ~masks[idx % 8];
        }
    }

    constexpr bool all() const {
        for (auto i: data)
            if (i != 0xFF)
                return false;
        return true;
    }

    constexpr bool any() const {
        for (auto i: data)
            if (i != 0)
                return true;
        return false;
    }

    constexpr bool none() const {
        return !any();
    }

#if __cpp_lib_is_constant_evaluated
    constexpr
#endif
            std::size_t
            count() const {
        std::size_t result = 0;
#if __cpp_lib_is_constant_evaluated
        if (std::is_constant_evaluated) {
            for (std::size_t i = 0; i < num_bits; ++i)
                result += test(i);
            return result;
        }
#endif
        // gets optimized to 'popcnt' instruction with the command line option '-march=haswell' (or any other supported architecture which has 'popcnt')
        // tested on gcc (>=9.1) and clang (>=3.6)
        auto count_bits = [](std::size_t x) {
            int res = 0;
            while (x) {
                res += 1;
                x = x & (x - 1);
            }
            return res;
        };
        
        for (std::size_t i = 0; i < num_bits / 64; ++i)
            result += count_bits(data.data.register_size_arr[i]);
        for (std::size_t i = (num_bits / 64) * 8; i < num_bits / 8; ++i)
            result += count_bits(data[i]);
        for (std::size_t i = (num_bits / 8) * 8; i < num_bits; ++i)
            result += test(i);

        return result;
    }

    constexpr std::size_t size() const {
        return num_bits;
    }

    constexpr small_bitset &operator&=(small_bitset const &other) {
        for (std::size_t i = 0; i < num_bytes; ++i)
            data[i] &= other.data[i];
        return *this;
    }

    constexpr small_bitset &operator|=(small_bitset const &other) {
        for (std::size_t i = 0; i < num_bytes; ++i)
            data[i] |= other.data[i];
        return *this;
    }

    constexpr small_bitset &operator^=(small_bitset const &other) {
        for (std::size_t i = 0; i < num_bytes; ++i)
            data[i] ^= other.data[i];
        return *this;
    }

    constexpr small_bitset operator~() const {
        small_bitset result = *this;
        result.flip();
        return result;
    }

    constexpr small_bitset &operator>>=(std::size_t amount) {
        if (amount >= 8) {
            for (std::size_t i = 0; i < num_bytes - amount / 8; ++i)
                data[i] = data[i + amount / 8];
            for (std::size_t i = num_bytes - amount / 8; i < num_bytes; ++i)
                data[i] = 0;
            amount %= 8;
        }
        for (std::size_t i = 0; i + 1 < num_bytes; ++i)
            data[i] = (data[i] >> amount) | (data[i + 1] << (8 - amount));
        data[num_bytes - 1] >>= amount;
        return *this;
    }

    constexpr small_bitset operator>>(std::size_t amount) const {
        small_bitset result = *this;
        result >>= amount;
        return result;
    }

    constexpr small_bitset &operator<<=(std::size_t amount) {
        if (amount >= 8) {
            for (std::size_t i = num_bytes; i-- > amount / 8;)
                data[i] = data[i - amount / 8];
            for (std::size_t i = amount / 8; i--;)
                data[i] = 0;
            amount %= 8;
        }
        for (std::size_t i = num_bytes; i-- > 1;)
            data[i] = (data[i] << amount) | (data[i - 1] << (8 - amount));
        data[0] <<= amount;
        return *this;
    }

    constexpr small_bitset operator<<(std::size_t amount) const {
        small_bitset result = *this;
        result <<= amount;
        return result;
    }

    constexpr void flip() {
        for (auto &i: data) i = ~i;
    }

    constexpr void set() {
        for (auto &i: data) i = 0xFF;
    }

    constexpr void reset() {
        for (auto &i: data) i = 0;
    }

    constexpr unsigned long to_ulong() const {
        unsigned long result = 0;
        for (std::size_t i = 0; i < num_bytes; ++i)
            result |= static_cast<unsigned long>(data[i]) << (8 * i);
        return result;
    }

    constexpr unsigned long long to_ullong() const {
        unsigned long long result = 0;
        for (std::size_t i = 0; i < num_bytes; ++i)
            result |= static_cast<unsigned long long>(data[i]) << (8 * i);
        return result;
    }

    constexpr std::string to_string() const {
        std::string res;
        res.reserve(num_bits);
        for (std::size_t i = 0; i < num_bits; ++i)
            res.push_back('0' + test(num_bits - i - 1));
        return res;
    }
}; // namespace sb

} // namespace sb

#endif