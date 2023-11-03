#pragma once

#include "Shared.h"

constexpr int INVALID_RAY_TYPE = -1;

enum class BufferToDisplay
{
    NoisyBeauty = 0,
    Albedo,
    Normal,
    Flow,
    DenoisedBeauty,
};

struct GAS
{
    optixu::GeometryAccelerationStructure gas;
    cudau::Buffer gasMem;
};

// Define the types of entry points as strings and as an enum
static const char* EntryPointTypeTable[] = {
    "debug",
    "pathtrace",
    "Invalid"};

// Define the structure for the EntryPointType
struct EntryPointType
{
    // Enum to list all possible entry points
    enum EEntryPointType
    {
        debug,
        pathtrace,
        Count,
        Invalid = Count
    };

    union
    {
        EEntryPointType name;
        unsigned int value;
    };

    // Constructors for initialization
    EntryPointType (EEntryPointType name) :
        name (name) {}
    EntryPointType (unsigned int value) :
        value (value) {}
    EntryPointType() :
        value (Invalid) {}

    // Conversion operator
    operator EEntryPointType() const { return name; }

    // Convert the entry point type to string
    const char* toString() const { return EntryPointTypeTable[value]; }

    // Convert a string to the corresponding entry point type
    static EntryPointType FromString (const char* str) { return mace::TableLookup (str, EntryPointTypeTable, Count); }
};

// Specialize std::hash for EntryPointType
namespace std
{
    template <>
    struct hash<EntryPointType>
    {
        size_t operator() (const EntryPointType& m) const
        {
            return std::hash<unsigned int>() (m.value);
        }
    };
} // namespace std

// Define the types of entry points as strings and as an enum
static const char* ProgramTypeTable[] = {
    "shading",
    "visibility",
    "miss",
    "Invalid"};

// Define the structure for the ProgramType
struct ProgramType
{
    // Enum to list all possible entry points
    enum EProgramType
    {
        shading,
        visibility,
        miss,
        Count,
        Invalid = Count
    };

    union
    {
        EProgramType name;
        unsigned int value;
    };

    // Constructors for initialization
    ProgramType (EProgramType name) :
        name (name) {}
    ProgramType (unsigned int value) :
        value (value) {}
    ProgramType() :
        value (Invalid) {}

    // Conversion operator
    operator EProgramType() const { return name; }

    // Convert the entry point type to string
    const char* toString() const { return ProgramTypeTable[value]; }

    // Convert a string to the corresponding entry point type
    static ProgramType FromString (const char* str) { return mace::TableLookup (str, ProgramTypeTable, Count); }
};

// Specialize std::hash for ProgramType
namespace std
{
    template <>
    struct hash<ProgramType>
    {
        size_t operator() (const ProgramType& m) const
        {
            return std::hash<unsigned int>() (m.value);
        }
    };
} // namespace std


// these replace OptiXUtility's compile time macros
// so we can create the shader kernels function name at runtime
inline std::string rt_raygen_name_str (const std::string& name)
{
    return "__raygen__" + name;
}

inline std::string rt_ch_name_str (const std::string& name)
{
    return "__closesthit__" + name;
}

inline std::string rt_ah_name_str (const std::string& name)
{
    return "__anyhit__" + name;
}

inline std::string rt_miss_name_str (const std::string& name)
{
    return "__miss__" + name;
}

struct PipelineData
{
    std::string rayGenName;
    std::string closestHitName;
    std::string anyHitName;
    std::string missName;
    int32_t numOfRayTypes;
    int32_t searchRay;
    int32_t visibilityRay;
    EntryPointType entryPoint;
    uint32_t numPayloadValuesInDwords = 0;
    uint32_t numAttributeValuesInDwords = 0;
    std::string plpName;
    size_t sizeOfLaunchParams = 0;
};


// Lookup table for Material Types as strings
static const char* MaterialTypeTable[] =
    {
        "Triangles",
        "Invalid"};

// Struct to encapsulate different material types
struct MaterialType
{
    // Enum for material types
    enum EMaterialType
    {
        Triangles,
        Count,          // Count of valid types
        Invalid = Count // Invalid is set to the Count value
    };

    // Union to hold either the material type enum or its integer value
    union
    {
        EMaterialType name;
        unsigned int value;
    };

    // Constructor to initialize MaterialType with EMaterialType
    MaterialType (EMaterialType name) :
        name (name) {}

    // Constructor to initialize MaterialType with an unsigned int
    MaterialType (unsigned int value) :
        value (value) {}

    // Default constructor sets type to Invalid
    MaterialType() :
        value (Invalid) {}

    // Operator overload to cast MaterialType to its enum value
    operator EMaterialType() const { return name; }

    // Convert the material type to its string representation
    const char* toString() const { return MaterialTypeTable[value]; }

    // Static function to create MaterialType from string
    static MaterialType FromString (const char* str) { return mace::TableLookup (str, MaterialTypeTable, Count); }
};

namespace std
{
    template <>
    struct hash<MaterialType>
    {
        size_t operator() (const MaterialType& m) const
        {
            return std::hash<unsigned int>() (m.value);
        }
    };
} // namespace std

struct MaterialInfo
{
    EntryPointType entryPoint = EntryPointType::Invalid;
    MaterialType materialType = MaterialType::Invalid;
    ProgramType shadingProg = ProgramType::Invalid;
    ProgramType visibilityProg = ProgramType::Invalid;
    int rayTypeSearch = INVALID_RAY_TYPE;
    int rayTypeVisibility = INVALID_RAY_TYPE;
};