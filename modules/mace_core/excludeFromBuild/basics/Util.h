/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

// NO MORE TRACING  Yay!
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

struct FileServices
{
    static void copyFiles (const std::string& searchFolder, const std::string& destFolder, const std::string& extension, bool recursive = true)
    {
        std::filesystem::recursive_directory_iterator dirIt (searchFolder), end;
        while (dirIt != end)
        {
            if (dirIt->path().extension() == extension || extension == "*")
            {
                std::filesystem::copy (dirIt->path(), destFolder + "/" + dirIt->path().filename().string());
            }
            ++dirIt;
        }
    }

    static void moveFiles (const std::string& searchFolder, const std::string& destFolder, const std::string& extension)
    {
        for (const auto& entry : std::filesystem::directory_iterator (searchFolder))
        {
            if (entry.path().extension() == extension || extension == "*")
            {
                std::filesystem::rename (entry, destFolder + "/" + entry.path().filename().string());
            }
        }
    }

    static std::vector<std::filesystem::path> findFilesWithExtension (const std::filesystem::path& searchFolder, const std::string extension)
    {
        std::vector<std::filesystem::path> matchingFiles;

        for (auto const& dir_entry : std::filesystem::directory_iterator{searchFolder})
        {
            if (dir_entry.path().extension().string() == extension)
            {
                matchingFiles.push_back (dir_entry.path());
            }
        }

        return matchingFiles;
    }

    static std::vector<std::string> getFiles (const std::filesystem::path& searchFolder, const std::string& extension, bool recursive)
    {
        std::vector<std::string> files;
        if (recursive)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator (searchFolder))
            {
                if (entry.path().extension() == extension || extension == ".*")
                {
                    files.push_back (entry.path().string());
                }
            }
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator (searchFolder))
            {
                if (entry.path().extension() == extension || extension == ".*")
                {
                    files.push_back (entry.path().string());
                }
            }
        }
        return files;
    }

    static std::vector<std::string> getFolders (const std::string& searchFolder, bool recursive = true)
    {
        std::vector<std::string> folders;
        std::filesystem::recursive_directory_iterator dirIt (searchFolder), end;
        while (dirIt != end)
        {
            if (std::filesystem::is_directory (dirIt->status()))
            {
                folders.push_back (dirIt->path().string());
            }
            ++dirIt;
        }
        return folders;
    }

    static std::vector<std::string> getTextFileLines (const std::string& filePath)
    {
        std::vector<std::string> lines;
        std::ifstream file (filePath);
        if (!file)
            return lines;

        std::string line;
        while (getline (file, line))
        {
            lines.push_back (line);
        }
        return lines;
    }

    static std::string findFilePath (const std::string& searchFolder, const std::string& fileName)
    {
        std::filesystem::recursive_directory_iterator dirIt (searchFolder), end;
        while (dirIt != end)
        {
            if (dirIt->path().filename() == fileName)
            {
                return dirIt->path().string();
            }
            ++dirIt;
        }
        return "";
    }

    static std::optional<std::filesystem::path> findFileInFolder (
        const std::filesystem::path& folder,
        const std::string& filename)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator (folder))
        {
            if (entry.path().filename() == filename)
            {
                return entry.path();
            }
        }
        return std::nullopt;
    }
};

inline bool hasObjExtension (const std::filesystem::path& filePath)
{
    return filePath.extension() == ".obj";
}

inline bool hasGltfExtension (const std::filesystem::path& filePath)
{
    return filePath.extension() == ".gltf";
}

inline bool isStaticBody (const std::filesystem::path& filePath)
{
    std::string filename = filePath.stem().string();

    // Check if filename starts with "static"
    return filename.rfind ("static", 0) == 0;
}

namespace mace
{
    // I think this is from InstantMeshes
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