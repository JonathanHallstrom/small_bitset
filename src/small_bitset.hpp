#ifndef SMALL_BITSET_H
#define SMALL_BITSET_H

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <string>

namespace sb {
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
                *byte |= 1 << which;
            } else {
                *byte &= ~(1 << which);
            }
            return *this;
        }

        constexpr bool operator~() const {
            return (*byte & (1 << which)) == 0;
        }

        constexpr operator bool() const {
            return (*byte & (1 << which)) != 0;
        }
    };

public:
    static constexpr std::size_t num_bytes = num_bits / 8 + (num_bits % 8 != 0);

    std::uint8_t data[num_bytes]{};

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
        return (data[idx / 8] >> idx % 8) & 1 != 0;
    }

    constexpr bool test(std::size_t idx) const {
        return (*this)[idx];
    }

    constexpr void set(std::size_t idx) {
        (*this)[idx] = true;
    }

    constexpr bool all() const {
        for (auto i: data)
            if (i != 0xFF)
                return false;
        return true;
    }

    constexpr bool any() const {
        for (auto i: data)
            if (i)
                return true;
        return false;
    }

    constexpr bool none() const {
        return !any();
    }

    constexpr std::size_t count() const {
        std::size_t result = 0;
        for (std::size_t i = 0; i < num_bytes - 1; ++i)
            result += std::bitset<8>(data[i]).count();
        for (std::size_t i = (num_bytes - 1) * 8; i < num_bits; ++i)
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
};

} // namespace small_bitset

#endif