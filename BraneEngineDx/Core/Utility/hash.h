#pragma once

#include <functional>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#if defined(_MSC_VER)
#   define FUNCTIONAL_HASH_ROTL32(x, r) _rotl(x,r)
#else
#   define FUNCTIONAL_HASH_ROTL32(x, r) (x << r) | (x >> (32 - r))
#endif

inline size_t hash_bytes(const unsigned char* first, size_t count) {
#if defined(_WIN64)
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    const size_t fnv_offset_basis = 14695981039346656037ULL;
    const size_t fnv_prime = 1099511628211ULL;
#else /* defined(_WIN64) */
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
    const size_t fnv_offset_basis = 2166136261U;
    const size_t fnv_prime = 16777619U;
#endif /* defined(_WIN64) */

    size_t result = fnv_offset_basis;
    for (size_t next = 0; next < count; ++next)
    {
        // fold in another byte
        result ^= (size_t)first[next];
        result *= fnv_prime;
    }
    return (result);
}

template <class T>
inline std::size_t hash_value_signed(T val)
{
    const unsigned int size_t_bits = std::numeric_limits<std::size_t>::digits;
    // ceiling(std::numeric_limits<T>::digits / size_t_bits) - 1
    const int length = (std::numeric_limits<T>::digits - 1)
        / static_cast<int>(size_t_bits);
    std::size_t seed = 0;
    T positive = val < 0 ? -1 - val : val;
    // Hopefully, this loop can be unrolled.
    for (unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
    {
        seed ^= (std::size_t)(positive >> i) + (seed << 6) + (seed >> 2);
    }
    seed ^= (std::size_t)val + (seed << 6) + (seed >> 2);
    return seed;
}

template <class T>
inline std::size_t hash_value_unsigned(T val)
{
    const unsigned int size_t_bits = std::numeric_limits<std::size_t>::digits;
    // ceiling(std::numeric_limits<T>::digits / size_t_bits) - 1
    const int length = (std::numeric_limits<T>::digits - 1)
        / static_cast<int>(size_t_bits);
    std::size_t seed = 0;
    // Hopefully, this loop can be unrolled.
    for (unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
    {
        seed ^= (std::size_t)(val >> i) + (seed << 6) + (seed >> 2);
    }
    seed ^= (std::size_t)val + (seed << 6) + (seed >> 2);
    return seed;
}

template <typename SizeT>
inline void hash_combine_impl(SizeT& seed, SizeT value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline void hash_combine_impl(uint32_t& h1, uint32_t k1)
{
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    k1 *= c1;
    k1 = FUNCTIONAL_HASH_ROTL32(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = FUNCTIONAL_HASH_ROTL32(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
}

inline void hash_combine_impl(uint64_t& h, uint64_t k)
{
    const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
    const int r = 47;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;

    // Completely arbitrary number, to prevent 0's
    // from hashing to 0.
    h += 0xe6546b64;
}

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
    std::hash<T> hasher;
    return hash_combine_impl(seed, hasher(v));
}

template <>
inline void hash_combine<size_t>(std::size_t& seed, std::size_t const& v)
{
    return hash_combine_impl(seed, v);
}

template <class It>
inline std::size_t hash_range(It first, It last)
{
    std::size_t seed = 0;

    for (; first != last; ++first)
    {
        hash_combine(seed, *first);
    }

    return seed;
}

template <class It>
inline void hash_range(std::size_t& seed, It first, It last)
{
    for (; first != last; ++first)
    {
        hash_combine(seed, *first);
    }
}

template <class T>
std::size_t hash_value(const T& v)
{
    std::size_t seed = 0;
    hash_combine(seed, v);
    return seed;
}

template <class A, class B>
std::size_t hash_value(std::pair<A, B> const& v)
{
    std::size_t seed = 0;
    hash_combine(seed, v.first);
    hash_combine(seed, v.second);
    return seed;
}

namespace std
{
    template <class A, class B>
    class hash<pair<A, B>>
    {
    public:
        size_t operator()(pair<A, B> const& v) const
        {
            return ::hash_value(v);
        }
    };
}

#define HASH_SPECIALIZE_ARRAY(type) \
template <class T, class A> \
inline std::size_t hash_value(const std::type<T, A>& v) \
{ \
    return hash_range(v.begin(), v.end()); \
}

#define HASH_SPECIALIZE_MAP(type) \
template <class K, class T, class C, class A> \
inline std::size_t hash_value(const std::type<K, T, C, A>& v) \
{ \
    return hash_range(v.begin(), v.end()); \
}

HASH_SPECIALIZE_ARRAY(vector);
HASH_SPECIALIZE_ARRAY(set);
HASH_SPECIALIZE_ARRAY(multiset);
HASH_SPECIALIZE_ARRAY(unordered_multiset);
HASH_SPECIALIZE_MAP(map);
HASH_SPECIALIZE_MAP(multimap);
HASH_SPECIALIZE_MAP(unordered_map);
HASH_SPECIALIZE_MAP(unordered_multimap);