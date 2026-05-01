#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace desktop::helpers::string_manip
{
	template<typename T>
	concept StringImplicitlyConstructible = std::is_constructible_v<T, std::string>&& std::is_convertible_v<std::string, T>;

	std::vector<std::byte> base64_decode(const std::string& input);
	std::string base64_encode(const std::vector<std::byte>& input);
	std::string filename_normalize(const std::string& filename, bool force_windows);
	std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
	std::string lower(const std::string& str);
	std::string quote(const std::string& str);
	std::string replace_all(const std::string& str, const std::string& from, const std::string& to);
	std::string replace_all(const std::string& str, char from, char to);
	std::string str(const std::wstring& wstr);
	std::string trim(const std::string& str);
	std::string trim(const std::string& str, char delimiter);
	std::string upper(const std::string& str);
	std::wstring wstr(const std::string& str);

    template<StringImplicitlyConstructible T = std::string>
    std::vector<T> split(const std::string& s, const std::string& delimiter, bool includeEmpty = true) noexcept
    {
        std::vector<T> splits;
        size_t last{ 0 };
        size_t next{ 0 };
        while ((next = s.find(delimiter, last)) != std::string::npos)
        {
            std::string token{ s.substr(last, next - last) };
            if (includeEmpty || !trim(token).empty())
            {
                splits.push_back(token);
            }
            last = next + delimiter.length();

        }
        std::string finalToken{ s.substr(last) };
        if (includeEmpty || !trim(finalToken).empty())
        {
            splits.push_back(finalToken);
        }
        return splits;
    }

    template<StringImplicitlyConstructible T = std::string>
    std::vector<T> split(const std::string& s, char delimiter, bool includeEmpty = true) noexcept
    {
        return split<T>(s, std::string(1, delimiter), includeEmpty);
    }
}