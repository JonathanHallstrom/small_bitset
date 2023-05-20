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

    static_assert(num_bits > 0, "number of bits has to be greater than zero");

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
            template<class... G> constexpr empty(G &&...) noexcept {}
            template<class G> constexpr empty &operator=(G &&) noexcept { return *this; }
            template<class G> constexpr bool operator==(G &&) const noexcept { return true; }
            template<class G> constexpr bool operator!=(G &&) const noexcept { return true; }
            constexpr operator std::size_t() const noexcept { return 0; }
        };
        union {
            empty register_size_arr[1]; // never accessed
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

    constexpr small_bitset &set(std::size_t idx) {
        data[idx / 8] |= masks[idx % 8];
        return *this;
    }

    constexpr small_bitset &reset(std::size_t idx) {
        data[idx / 8] &= ~masks[idx % 8];
        return *this;
    }

    constexpr void set(std::size_t idx, bool value) {
        if (value) {
            data[idx / 8] |= masks[idx % 8];
        } else {
            data[idx / 8] &= ~masks[idx % 8];
        }
    }

    constexpr bool all() const {
        bool result = true;
        _apply_to_all_const([&result](auto x) { result &= x == (typename std::remove_reference<decltype(x)>::type)(-1); });
        constexpr std::uint8_t last_byte_masks[] = {0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111};
        result &= (last_byte_masks[num_bits % 8] & ~data[num_bytes - 1]) == 0;
        return result;
    }

    constexpr bool any() const {
        bool result = false;
        _apply_to_all_const([&result](auto x) { result |= x != 0; });
        constexpr std::uint8_t last_byte_masks[] = {0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111};
        result |= (last_byte_masks[num_bits % 8] & data[num_bytes - 1]) != 0;
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
#if defined(__clang__) || defined(__GNUC__) || __has_builtin(__builtin_popcountll)
            return __builtin_popcountll(x);
#endif
            int res = 0;
            while (x) {
                res += 1;
                x = x & (x - 1);
            }
            return res;
        };

        _apply_to_all_const([count_bits, &result](auto x) {
            result += count_bits(x);
        });

        constexpr std::uint8_t last_byte_masks[] = {0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111};
        result += count_bits(last_byte_masks[num_bits % 8] & data[num_bytes - 1]);

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
        return result.flip();
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

    constexpr small_bitset &flip() {
        _apply_to_all([](auto &x) { x = ~x; }, true);
        return *this;
    }

    constexpr void set() {
        _apply_to_all([](auto &x) { x = (typename std::remove_reference<decltype(x)>::type)(-1); }, true);
    }

    constexpr void reset() {
        _apply_to_all([](auto &x) { x = 0; }, true);
    }

    /*
     * bits which wouldn't fit in an unsigned long are ignored
     */
    constexpr unsigned long to_ulong() const {
        unsigned long result = 0;
        for (std::size_t i = 0; i < std::min(sizeof(unsigned long), num_bytes); ++i)
            result |= static_cast<unsigned long>(data[i]) << (8 * i);
        return result;
    }

    /*
     * bits which wouldn't fit in an unsigned long long are ignored
     */
    constexpr unsigned long long to_ullong() const {
        unsigned long long result = 0;
        for (std::size_t i = 0; i < std::min(sizeof(unsigned long long), num_bytes); ++i)
            result |= static_cast<unsigned long long>(data[i]) << (8 * i);
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
    template<class F>
    constexpr void _apply_to_all(F &&func_obj, bool include_last_partial_byte = false) {
        constexpr std::size_t register_bytes = sizeof(std::size_t);
        constexpr std::size_t register_bits = register_bytes * 8;

        if (num_bytes >= 8)
            for (auto &i: data.data.register_size_arr)
                func_obj(i);

        if (include_last_partial_byte)
            for (std::size_t i = (num_bits / register_bits) * register_bytes; i < num_bytes; ++i)
                func_obj(data[i]);
        else
            for (std::size_t i = (num_bits / register_bits) * register_bytes; i < num_bits / 8; ++i)
                func_obj(data[i]);
    }

    template<class F>
    constexpr void _apply_to_all_const(F &&func_obj, bool include_last_partial_byte = false) const {
        constexpr std::size_t register_bytes = sizeof(std::size_t);
        constexpr std::size_t register_bits = register_bytes * 8;

        if (num_bytes >= 8)
            for (auto &i: data.data.register_size_arr)
                func_obj(i);

        if (include_last_partial_byte)
            for (std::size_t i = (num_bits / register_bits) * register_bytes; i < num_bytes; ++i)
                func_obj(data[i]);
        else
            for (std::size_t i = (num_bits / register_bits) * register_bytes; i < num_bits / 8; ++i)
                func_obj(data[i]);
    }
};

} // namespace sb

#endif