#include "helpers/string_manip.h"
#include <algorithm>
#include <cwchar>
#include <limits>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

namespace desktop::helpers
{
    std::vector<std::byte> string_manip::base64_decode(const std::string& input)
    {
        if (input.empty() || input.size() % 4 != 0)
        {
            return {};
        }
        static const unsigned char lookup[128]{
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255,  62, 255,  63,
             52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255,
            255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
             15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255,  63,
            255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
             41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255
        };
        std::vector<std::byte> bytes;
        bytes.reserve(3 * input.size() / 4);
        for (size_t i = 0; i < input.size(); i += 4)
        {
            unsigned char b641{ input[i] <= 122 ? lookup[static_cast<unsigned char>(input[i])] : static_cast<unsigned char>(0xff) };
            unsigned char b642{ input[i + 1] <= 122 ? lookup[static_cast<unsigned char>(input[i + 1])] : static_cast<unsigned char>(0xff) };
            unsigned char b643{ input[i + 2] <= 122 ? lookup[static_cast<unsigned char>(input[i + 2])] : static_cast<unsigned char>(0xff) };
            unsigned char b644{ input[i + 3] <= 122 ? lookup[static_cast<unsigned char>(input[i + 3])] : static_cast<unsigned char>(0xff) };
            if (b642 != 0xff)
            {
                bytes.push_back(static_cast<std::byte>(((b641 & 0x3f) << 2) + ((b642 & 0x30) >> 4)));
            }
            if (b643 != 0xff)
            {
                bytes.push_back(static_cast<std::byte>(((b642 & 0x0f) << 4) + ((b643 & 0x3c) >> 2)));
            }
            if (b644 != 0xff)
            {
                bytes.push_back(static_cast<std::byte>(((b643 & 0x03) << 6) + (b644 & 0x3f)));
            }
        }
        return bytes;
    }

    std::string string_manip::base64_encode(const std::vector<std::byte>& input)
    {
        if (input.empty())
        {
            return "";
        }
        static const char lookup[65]{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
        size_t missing{ 0 };
        size_t paddedSize{ input.size() };
        while (paddedSize % 3 != 0)
        {
            paddedSize++;
            missing++;
        }
        size_t stringSize{ 4 * paddedSize / 3 };
        std::string result;
        result.reserve(stringSize);
        for (size_t i = 0; i < stringSize / 4; i++)
        {
            size_t idx{ i * 3 };
            unsigned char b1{ idx < input.size() ? static_cast<unsigned char>(input[idx]) : static_cast<unsigned char>(0) };
            unsigned char b2{ idx + 1 < input.size() ? static_cast<unsigned char>(input[idx + 1]) : static_cast<unsigned char>(0) };
            unsigned char b3{ idx + 2 < input.size() ? static_cast<unsigned char>(input[idx + 2]) : static_cast<unsigned char>(0) };
            result.push_back(lookup[(b1 & 0xfc) >> 2]);
            result.push_back(lookup[((b1 & 0x03) << 4) + ((b2 & 0xf0) >> 4)]);
            result.push_back(lookup[((b2 & 0x0f) << 2) + ((b3 & 0xc0) >> 6)]);
            result.push_back(lookup[b3 & 0x3f]);
        }
        for (size_t i = 0; i < missing; i++)
        {
            result[stringSize - i - 1] = '=';
        }
        return result;
    }

    std::string string_manip::filename_normalize(const std::string& filename, bool force_windows)
    {
        std::string result{ replace_all(filename, '/', '_') };
#ifdef _WIN32
        const bool is_windows{ true };
#else
        const bool is_windows{ force_windows };
#endif
        if (is_windows)
        {
            result = replace_all(result, '<', '_');
            result = replace_all(result, '>', '_');
            result = replace_all(result, ':', '_');
            result = replace_all(result, '"', '_');
            result = replace_all(result, '\\', '_');
            result = replace_all(result, '|', '_');
            result = replace_all(result, '?', '_');
            result = replace_all(result, '*', '_');
        }
        return result;
    }

    std::string string_manip::join(const std::vector<std::string>& strings, const std::string& delimiter)
    {
        std::ostringstream builder;
        for (size_t i = 0; i < strings.size(); i++)
        {
            builder << strings[i];
            if (i != strings.size() - 1)
            {
                builder << delimiter;
            }
        }
        return builder.str();
    }

    std::string string_manip::lower(const std::string& str)
    {
        std::string result{ str };
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::string string_manip::quote(const std::string& str)
    {
        if (str.empty())
        {
            return "\"\"";
        }
        if (str.front() == '"' && str.back() == '"')
        {
            return str;
        }
#ifndef _WIN32
        if (str.front() == '\'' && str.back() == '\'')
        {
            return str;
        }
        if (str.find('"') != std::string::npos)
        {
            return '\'' + str + '\'';
        }
#endif
        return '"' + str + '"';
    }

    std::string string_manip::replace_all(const std::string& str, const std::string& from, const std::string& to)
    {
        if (str.empty() || from.empty())
        {
            return str;
        }
        std::string result{ str };
        size_t pos{ 0 };
        while ((pos = result.find(from, pos)) != std::string::npos)
        {
            result.replace(pos, from.size(), to);
            pos += to.size();
        }
        return result;
    }

    std::string string_manip::replace_all(const std::string& str, char from, char to)
    {
        std::string result{ str };
        std::replace(result.begin(), result.end(), from, to);
        return result;
    }

    std::string string_manip::str(const std::wstring& wstr)
    {
#ifdef _WIN32
        int size{ WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr) };
        if (size <= 0)
        {
            return {};
        }
        std::string result(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, result.data(), size, nullptr, nullptr);
        return result;
#else
        std::mbstate_t state{};
        const wchar_t* ptr{ wstr.data() };
        size_t size{ 1 + std::wcsrtombs(nullptr, &ptr, 0, &state) };
        if (size == static_cast<size_t>(-1))
        {
            return {};
        }
        std::vector<char> buf(size);
        std::wcsrtombs(buf.data(), &ptr, buf.size(), &state);
        return std::string(buf.data());
#endif
    }

    std::string string_manip::trim(const std::string& str)
    {
        if (str.empty())
        {
            return str;
        }
        std::string result{ str };
        result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }).base(), result.end());
        result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch)
            {
                return !std::isspace(ch);
            }));
        return result;
    }

    std::string string_manip::trim(const std::string& str, char delimiter)
    {
        if (str.empty())
        {
            return str;
        }
        std::string result{ str };
        result.erase(std::find_if(result.rbegin(), result.rend(), [delimiter](char ch)
            {
                return ch != delimiter;
            }).base(), result.end());
        result.erase(result.begin(), std::find_if(result.begin(), result.end(), [delimiter](char ch)
            {
                return ch != delimiter;
            }));
        return result;
    }

    std::string string_manip::upper(const std::string& str)
    {
        std::string result{ str };
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
        return result;
    }

    std::wstring string_manip::wstr(const std::string& str)
    {
#ifdef _WIN32
        int size{ MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0) };
        if (size <= 0)
        {
            return {};
        }
        std::wstring result(size - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size);
        return result;
#else
        std::mbstate_t state{};
        const char* ptr{ str.data() };
        size_t size{ 1 + std::mbsrtowcs(nullptr, &ptr, 0, &state) };
        if (size == static_cast<size_t>(-1))
        {
            return {};
        }
        std::vector<wchar_t> buf(size);
        std::mbsrtowcs(buf.data(), &ptr, buf.size(), &state);
        return std::wstring(buf.data());
#endif
    }
}