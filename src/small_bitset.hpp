#ifndef SMALL_BITSET_H
#define SMALL_BITSET_H

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>

#if __cplusplus >= 201703L
#define CXX17CONSTEXPR constexpr
#else
#define CXX17CONSTEXPR
#endif

namespace sb {
static constexpr std::uint8_t masks[] = {1, 2, 4, 8, 16, 32, 64, 128};
/// same API as std::bitset with the same functionality
/// except of course the fact that this uses the least memory possible
/// note: shifting can be a little slow
template<std::size_t num_bits>
class small_bitset {
    constexpr static std::size_t REGISTER_BYTES = sizeof(std::size_t);
    constexpr static std::size_t BITS_PER_BYTE = 8;
    constexpr static std::size_t REGISTER_BITS = REGISTER_BYTES * 8;
    constexpr static std::size_t LAST_BYTE_MASK = 0xFF >> (BITS_PER_BYTE - num_bits % BITS_PER_BYTE) % BITS_PER_BYTE;

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
#define NUM_BYTES (num_bits / BITS_PER_BYTE + (num_bits % BITS_PER_BYTE != 0))

    static_assert(num_bits > 0, "number of bits has to be greater than zero");

    struct big_version {
        union {
            std::size_t register_size_arr[NUM_BYTES / REGISTER_BYTES];
            std::uint8_t byte_size_arr[NUM_BYTES]{};
        } data;

        constexpr std::uint8_t &operator[](std::size_t idx) {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t const &operator[](std::size_t idx) const {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t *begin() {
            return &data.byte_size_arr[0];
        }

        constexpr std::uint8_t const *begin() const {
            return &data.byte_size_arr[0];
        }

        constexpr std::uint8_t *end() {
            return &data.byte_size_arr[NUM_BYTES];
        }

        constexpr std::uint8_t const *end() const {
            return &data.byte_size_arr[NUM_BYTES];
        }
    };

    struct small_version {
        struct empty {
            template<class... G> constexpr empty(G &&...) noexcept {}
            template<class G> constexpr empty &operator=(G &&) noexcept { return *this; }
            template<class G> constexpr bool operator==(G &&) const noexcept { return 0; }
            template<class G> constexpr bool operator!=(G &&) const noexcept { return true; }
            constexpr operator std::size_t() const noexcept { return 0; }
        };
        union {
            empty register_size_arr[1]; // never accessed
            std::uint8_t byte_size_arr[NUM_BYTES]{};
        } data;

        constexpr std::uint8_t &operator[](std::size_t idx) {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t const &operator[](std::size_t idx) const {
            return data.byte_size_arr[idx];
        }

        constexpr std::uint8_t *begin() {
            return &data.byte_size_arr[0];
        }

        constexpr std::uint8_t const *begin() const {
            return &data.byte_size_arr[0];
        }

        constexpr std::uint8_t *end() {
            return &data.byte_size_arr[NUM_BYTES];
        }

        constexpr std::uint8_t const *end() const {
            return &data.byte_size_arr[NUM_BYTES];
        }
    };

    typename std::conditional<(NUM_BYTES >= REGISTER_BYTES), big_version, small_version>::type data{};

    constexpr small_bitset() = default;

    constexpr small_bitset(small_bitset const &other) = default;

    constexpr small_bitset &operator=(small_bitset const &other) = default;

    constexpr small_bitset(std::uint64_t u) {
        std::size_t i = 0;
        while (u) {
            assert(i < NUM_BYTES && "u does not fit in bitset");
            data[i++] = u & 0xff;
            u >>= BITS_PER_BYTE;
        }
    }

    constexpr bool operator==(small_bitset other) const {
        for (std::size_t i = 0; i < NUM_BYTES; ++i)
            if (data[i] != other.data[i])
                return false;
        return true;
    }

    constexpr bool operator!=(small_bitset other) const {
        return !(*this == other);
    }

    constexpr bit_ref operator[](std::size_t idx) {
        return bit_ref{data[idx / BITS_PER_BYTE], idx % BITS_PER_BYTE};
    }

    constexpr bool operator[](std::size_t idx) const {
        return (data[idx / BITS_PER_BYTE] & masks[idx % BITS_PER_BYTE]) != 0;
    }

    constexpr bool test(std::size_t idx) const {
        return (data[idx / BITS_PER_BYTE] & masks[idx % BITS_PER_BYTE]) != 0;
    }

    constexpr small_bitset &set(std::size_t idx) {
        data[idx / BITS_PER_BYTE] |= masks[idx % BITS_PER_BYTE];
        return *this;
    }

    constexpr small_bitset &reset(std::size_t idx) {
        data[idx / BITS_PER_BYTE] &= ~masks[idx % BITS_PER_BYTE];
        return *this;
    }

    constexpr small_bitset &set(std::size_t idx, bool value) {
        if (value) {
            data[idx / BITS_PER_BYTE] |= masks[idx % BITS_PER_BYTE];
        } else {
            data[idx / BITS_PER_BYTE] &= ~masks[idx % BITS_PER_BYTE];
        }
        return *this;
    }

    constexpr bool all() const {
        bool result = true;
        _apply_to_all_const([&result](auto x, auto mask) {
            result &= (x & mask) == mask;
        });
        return result;
    }

    constexpr bool any() const {
        bool result = false;
        _apply_to_all_const([&result](auto x, auto mask) {
            result |= (x & mask) != 0;
        });
        return result;
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
        if (std::is_constant_evaluated()) {
            for (std::size_t i = 0; i < num_bits; ++i)
                result += test(i);
            return result;
        }
#endif
        auto count_bits = [](std::size_t x) {
#if defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)
            return __builtin_popcountll(x);
#endif
            int res = 0;
            while (x) {
                res += 1;
                x = x & (x - 1);
            }
            return res;
        };

        _apply_to_all_const([count_bits, &result](std::size_t x, std::size_t mask) {
            result += count_bits(x & mask);
        });

        return result;
    }

    constexpr std::size_t size() const {
        return num_bits;
    }

    CXX17CONSTEXPR small_bitset &operator&=(small_bitset const &other) {
        for (std::size_t i = 0; i < NUM_BYTES; ++i)
            data[i] &= other.data[i];
        return *this;
    }

    CXX17CONSTEXPR small_bitset &operator|=(small_bitset const &other) {
        for (std::size_t i = 0; i < NUM_BYTES; ++i)
            data[i] |= other.data[i];
        return *this;
    }

    CXX17CONSTEXPR small_bitset &operator^=(small_bitset const &other) {
        for (std::size_t i = 0; i < NUM_BYTES; ++i)
            data[i] ^= other.data[i];
        return *this;
    }

    CXX17CONSTEXPR small_bitset operator&(small_bitset const &other) const {
        small_bitset result = *this;
        result &= other;
        return result;
    }

    CXX17CONSTEXPR small_bitset operator|(small_bitset const &other) const {
        small_bitset result = *this;
        result |= other;
        return result;
    }

    CXX17CONSTEXPR small_bitset operator^(small_bitset const &other) const {
        small_bitset result = *this;
        result ^= other;
        return result;
    }

    CXX17CONSTEXPR small_bitset operator~() const {
        small_bitset result = *this;
        return result.flip();
    }

    CXX17CONSTEXPR small_bitset &operator>>=(std::size_t amount) {
        data[NUM_BYTES - 1] &= static_cast<std::uint8_t>(0xFF) >> ((BITS_PER_BYTE - num_bits % BITS_PER_BYTE) % BITS_PER_BYTE);
        if (amount >= BITS_PER_BYTE) {
            for (std::size_t i = 0; i < NUM_BYTES - amount / BITS_PER_BYTE; ++i)
                data[i] = data[i + amount / BITS_PER_BYTE];
            for (std::size_t i = NUM_BYTES - amount / BITS_PER_BYTE; i < NUM_BYTES; ++i)
                data[i] = 0;
            amount %= BITS_PER_BYTE;
        }
        if (amount) {
            for (std::size_t i = 0; i + 1 < NUM_BYTES; ++i)
                data[i] = (data[i] >> amount) | (data[i + 1] << (BITS_PER_BYTE - amount));
            data[NUM_BYTES - 1] >>= amount;
        }
        return *this;
    }

    CXX17CONSTEXPR small_bitset operator>>(std::size_t amount) const {
        small_bitset result = *this;
        result >>= amount;
        return result;
    }

    CXX17CONSTEXPR small_bitset &operator<<=(std::size_t amount) {
        if (amount >= BITS_PER_BYTE) {
            for (std::size_t i = NUM_BYTES; i-- > amount / BITS_PER_BYTE;)
                data[i] = data[i - amount / BITS_PER_BYTE];
            for (std::size_t i = amount / BITS_PER_BYTE; i--;)
                data[i] = 0;
            amount %= BITS_PER_BYTE;
        }
        if (amount) {
            for (std::size_t i = NUM_BYTES; i-- > 1;)
                data[i] = (data[i] << amount) | (data[i - 1] >> (BITS_PER_BYTE - amount));
            data[0] <<= amount;
        }
        _fix_last_byte();
        return *this;
    }

    constexpr small_bitset operator<<(std::size_t amount) const {
        small_bitset result = *this;
        result <<= amount;
        return result;
    }

    CXX17CONSTEXPR small_bitset &flip() {
        _modify_all_bytes([](auto &x) { x = ~x; });
        _fix_last_byte();
        return *this;
    }

    CXX17CONSTEXPR small_bitset &set() {
        _modify_all_bytes([](auto &x) { x = (typename std::remove_reference<decltype(x)>::type)(-1); });
        _fix_last_byte();
        return *this;
    }

    CXX17CONSTEXPR small_bitset &reset() {
        _modify_all_bytes([](auto &x) { x = 0; });
        return *this;
    }

    /*
     * bits which wouldn't fit in an unsigned long are ignored
     * this differs from the standard version which throws an exception
     */
#if __cpp_lib_is_constant_evaluated
    constexpr
#endif
            unsigned long
            to_ulong() const {
        unsigned long result = 0;

#if __cpp_lib_is_constant_evaluated
        if (!std::is_constant_evaluated()) {
            std::memcpy(&result, data.begin(), std::min(sizeof(unsigned long), NUM_BYTES));
        } else {
            for (std::size_t i = 0; i < std::min(sizeof(unsigned long), NUM_BYTES); ++i)
                result |= static_cast<unsigned long>(data[i]) << (BITS_PER_BYTE * i);
        }
#else
        std::memcpy(&result, data.begin(), std::min(sizeof(unsigned long), NUM_BYTES));
#endif
        return result;
    }

    /*
     * bits which wouldn't fit in an unsigned long long are ignored
     * this differs from the standard version which throws an exception
     */
#if __cpp_lib_is_constant_evaluated
    constexpr
#endif
            unsigned long long
            to_ullong() const {
        unsigned long long result = 0;

#if __cpp_lib_is_constant_evaluated
        if (!std::is_constant_evaluated()) {
            std::memcpy(&result, data.begin(), std::min(sizeof(unsigned long long), NUM_BYTES));
        } else {
            for (std::size_t i = 0; i < std::min(sizeof(unsigned long long), NUM_BYTES); ++i)
                result |= static_cast<unsigned long>(data[i]) << (BITS_PER_BYTE * i);
        }
#else
        std::memcpy(&result, data.begin(), std::min(sizeof(unsigned long long), NUM_BYTES));
#endif
        return result;
    }

#if __cplusplus >= 202002l
    constexpr
#endif
            std::string
            to_string() const {
        std::string res;
        res.reserve(num_bits);
        for (std::size_t i = 0; i < num_bits; ++i)
            res.push_back('0' + test(num_bits - i - 1));
        return res;
    }

private:
    CXX17CONSTEXPR void _fix_last_byte() {
        data[NUM_BYTES - 1] &= LAST_BYTE_MASK;
    }

    template<class F>
    CXX17CONSTEXPR void _modify_all_bytes(F &&func_obj) {

        if (NUM_BYTES >= REGISTER_BYTES)
            for (auto &i: data.data.register_size_arr)
                func_obj(i);
        if (NUM_BYTES % REGISTER_BYTES == 0) return;

        for (std::size_t i = (num_bits / REGISTER_BITS) * REGISTER_BYTES; i < NUM_BYTES; ++i)
            func_obj(data[i]);
        _fix_last_byte();
    }

    template<class F>
    CXX17CONSTEXPR void _apply_to_all_const(F &&func_obj) const {

        if (NUM_BYTES >= REGISTER_BYTES) {
            if (NUM_BYTES % REGISTER_BYTES == 0 && num_bits % REGISTER_BITS) {
                for (std::size_t i = 0; i < NUM_BYTES / REGISTER_BYTES - 1; ++i)
                    func_obj(data.data.register_size_arr[i], static_cast<std::size_t>(-1));

                func_obj(data.data.register_size_arr[NUM_BYTES / REGISTER_BYTES - 1], static_cast<std::size_t>(-1) >> (REGISTER_BITS - num_bits % REGISTER_BITS));
                return;
            } else {
                for (std::size_t i = 0; i < NUM_BYTES / REGISTER_BYTES; ++i)
                    func_obj(data.data.register_size_arr[i], static_cast<std::size_t>(-1));
            }
        }
        std::size_t temp = 0;
        std::size_t mask = 0;
        std::size_t shift = 0;
        for (std::size_t i = (num_bits / REGISTER_BITS) * BITS_PER_BYTE; i < num_bits / BITS_PER_BYTE; ++i)
            temp |= static_cast<std::size_t>(data[i]) << shift, mask |= 0xFFull << shift, shift += BITS_PER_BYTE;

        if (num_bits % BITS_PER_BYTE) {
            temp |= static_cast<std::size_t>(data[NUM_BYTES - 1]) << shift, mask |= static_cast<std::size_t>(LAST_BYTE_MASK) << shift, shift += BITS_PER_BYTE;
        }
        if (shift)
            func_obj(temp, mask);
    }
};

} // namespace sb

#endif