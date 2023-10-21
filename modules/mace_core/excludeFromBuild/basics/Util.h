// This header file was auto-generated by ClassMate++
// Created: 22 Feb 2019 8:55:26 pm
// Copyright (c) 2019, HurleyWorks

#pragma once

// NO MORE TRACING  Yay
#define TRACE(x)

const LEVELS TESTING{INFO.value + 1, "TESTING"};
const LEVELS CRITICAL{WARNING.value + 1, "CRTICAL"};

#if defined(_WIN32) || defined(_WIN64)
#define __FUNCTION_NAME__ __func__
#define _FN_ __FUNCTION_NAME__
#ifndef NOMINMAX
#define NOMINMAX
#endif

#undef near
#undef far
#undef RGB
#endif

// https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
//  https://gist.github.com/cbsmith/5538174
template <typename RandomGenerator = std::default_random_engine>
struct random_selector
{
    // On most platforms, you probably want to use std::random_device("/dev/urandom")()
    random_selector (RandomGenerator g = RandomGenerator (std::random_device()())) :
        gen (g) {}

    template <typename Iter>
    Iter select (Iter start, Iter end)
    {
        std::uniform_int_distribution<> dis (0, std::distance (start, end) - 1);
        std::advance (start, dis (gen));
        return start;
    }

    // convenience function
    template <typename Iter>
    Iter operator() (Iter start, Iter end)
    {
        return select (start, end);
    }

    // convenience function that works on anything with a sensible begin() and end(), and returns with a ref to the value type
    template <typename Container>
    auto operator() (const Container& c) -> decltype (*begin (c))&
    {
        return *select (begin (c), end (c));
    }

 private:
    RandomGenerator gen;
};

// makes it illegal to copy a derived class
// https://github.com/heisters/libnodes
struct Noncopyable
{
 protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable (const Noncopyable&) = delete;
    Noncopyable& operator= (const Noncopyable&) = delete;
};

// provides derived classes with automatically assigned,
// globally unique numeric identifiers
// https://github.com/heisters/libnodes
class HasId
{
 public:
    HasId() :
        mId (++sId)
    {
        // LOG (DBUG) << mId;
    }

    ItemID id() const { return mId; }
    void setID (ItemID itemID) { mId = itemID; }

    void staticReset (int id = 0) { sId = id; }

 protected:
    static ItemID sId;
    ItemID mId;
};

// from the Code Blacksmith
// https://www.youtube.com/watch?v=GV0JMHOpoEw
class ScopedStopWatch
{
 public:
    using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                     std::chrono::high_resolution_clock,
                                     std::chrono::steady_clock>;
    ScopedStopWatch (const char function[] = "unknown function") :
        func (function)
    {
    }
    ~ScopedStopWatch()
    {
        LOG (DBUG) << "\n"
                   << func << " took " << std::chrono::duration_cast<std::chrono::milliseconds> (Clock::now() - start).count() << " milliseconds";
    }

 private:
    const char* func = nullptr;
    Clock::time_point start = Clock::now();
};

// store and retrieve any type from a map
template <class PROPERTY>
class AnyValue
{
    using ValueMap = std::unordered_map<int, std::any>;

 public:
    AnyValue() = default;
    ~AnyValue() = default;

    void addDefault (const PROPERTY& key, const std::any& value) { map_.insert (std::make_pair (key, value)); }
    void setValue (const PROPERTY& key, const std::any& value)
    {
        auto it = map_.find (key);
        if (it == map_.end())
            map_.insert (std::make_pair (key, value));
        else
            it->second = value;
    }

    template <typename T>
    T& getRef (const PROPERTY& key) { return std::any_cast<T> (getValue (key)); }

    template <typename T>
    T getVal (const PROPERTY& key) { return std::any_cast<T> (getValue (key)); }

    template <typename T>
    T* getPtr (const PROPERTY& key) { return std::any_cast<T> (&getValue (key)); }

 private:
    ValueMap map_;
    std::any empty_;

    std::any& getValue (const PROPERTY& key)
    {
        auto it = map_.find (key);
        if (it != map_.end())
            return it->second;

        return empty_;
    }

}; // end class AnyValue

inline double generateRandomDouble (double lower_bound, double upper_bound)
{
    std::random_device rd;                                             // Obtain a random number from hardware
    std::mt19937 gen (rd());                                           // Seed the generator
    std::uniform_real_distribution<> distr (lower_bound, upper_bound); // Define the range

    return distr (gen); // Generate numbers
}

inline double randomUniform (double min, double max)
{
    std::random_device rd;
    std::default_random_engine generator (rd());
    std::uniform_real_distribution<double> distribution (min, max);

    return distribution (generator);
}

inline double generateRandomDouble()
{
    static std::mt19937 generator (12345);              // A Mersenne Twister pseudo-random generator with a seed of 12345
    std::uniform_real_distribution<> distr (-1.0, 1.0); // Define the distribution between -1.0 and 1.0

    return distr (generator);
}

inline std::filesystem::path changeFileExtensionToJpeg (const std::filesystem::path& pngPath)
{
    std::filesystem::path newPath = pngPath;
    newPath.replace_extension (".jpeg");
    return newPath;
}

inline std::string readTxtFile (const std::filesystem::path& filepath)
{
    std::ifstream ifs;
    ifs.open (filepath, std::ios::in);
    if (ifs.fail())
        return "";

    std::stringstream sstream;
    sstream << ifs.rdbuf();

    return std::string (sstream.str());
}

inline std::vector<char> readBinaryFile (const std::filesystem::path& filepath)
{
    std::vector<char> ret;

    std::ifstream ifs;
    ifs.open (filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (ifs.fail())
        return std::move (ret);

    std::streamsize fileSize = ifs.tellg();
    ifs.seekg (0, std::ios::beg);

    ret.resize (fileSize);
    ifs.read (ret.data(), fileSize);

    return std::move (ret);
}

namespace mace
{

    inline bool atomicCompareAndExchange (volatile uint32_t* v, uint32_t newValue, uint32_t oldValue)
    {
#if defined(_WIN32)
        return _InterlockedCompareExchange (
                   reinterpret_cast<volatile long*> (v), (long)newValue, (long)oldValue) == (long)oldValue;
#else
        return __sync_bool_compare_and_swap (v, oldValue, newValue);
#endif
    }

    inline uint32_t atomicAdd (volatile uint32_t* dst, uint32_t delta)
    {
#if defined(_MSC_VER)
        return _InterlockedExchangeAdd (reinterpret_cast<volatile long*> (dst), delta) + delta;
#else
        return __sync_add_and_fetch (dst, delta);
#endif
    }

    inline float atomicAdd (volatile float* dst, float delta)
    {
        union bits
        {
            float f;
            uint32_t i;
        };
        bits oldVal, newVal;
        do
        {
#if defined(__i386__) || defined(__amd64__)
            __asm__ __volatile__ ("pause\n");
#endif
            oldVal.f = *dst;
            newVal.f = oldVal.f + delta;
        } while (!atomicCompareAndExchange ((volatile uint32_t*)dst, newVal.i, oldVal.i));
        return newVal.f;
    }
} // namespace mace