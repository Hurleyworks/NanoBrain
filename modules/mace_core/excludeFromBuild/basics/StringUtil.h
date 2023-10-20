#pragma once

// convert string into hex value
template <typename T>
inline T string2HexVal (const std::string& s)
{
    T value;
    std::stringstream ss;
    ss << s;
    ss >> std::hex >> value;

    return value;
}

// convert built-in type into a string
template <class T>
inline std::string ToString (T value)
{
    std::ostringstream buffer;
    buffer.setf (std::ios::fixed, std::ios::floatfield);
    buffer.precision (8);

    buffer << value;

    return buffer.str().c_str();
}

// convert string to a built-in type
template <typename T>
inline T StringToValue (const std::string& s)
{
    T value;
    std::stringstream ss;
    ss << s;
    ss >> value;

    return value;
}

inline std::string str_toupper (std::string s)
{
    std::transform (s.begin(), s.end(), s.begin(),
                    // static_cast<int(*)(int)>(std::toupper)         // wrong
                    // [](int c){ return std::toupper(c); }           // wrong
                    // [](char c){ return std::toupper(c); }          // wrong
                    [] (unsigned char c)
                    { return std::toupper (c); } // correct
    );
    return s;
}

// TableLookup
inline int TableLookup (const char* str, const char* table[], unsigned int count)
{
    const unsigned int invalid = count;
    if (!str)
        return invalid;
    while (count--)
        if (!strcmp (table[count], str))
            return count;

    return invalid;
}



