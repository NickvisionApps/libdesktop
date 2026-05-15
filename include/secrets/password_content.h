#pragma once

#include <cstdint>
#include <type_traits>

namespace desktop::secrets
{
	enum class password_content : std::uint8_t
	{
		numeric = 1,
		uppercase = 2,
		lowercase = 4,
		special = 8,
		space = 16
	};

	inline password_content operator|(password_content a, password_content b) noexcept
	{
		return static_cast<password_content>(static_cast<std::underlying_type_t<password_content>>(a) |
		                                     static_cast<std::underlying_type_t<password_content>>(b));
	}

	inline password_content operator&(password_content a, password_content b) noexcept
	{
		return static_cast<password_content>(static_cast<std::underlying_type_t<password_content>>(a) &
		                                     static_cast<std::underlying_type_t<password_content>>(b));
	}

	inline password_content operator^(password_content a, password_content b) noexcept
	{
		return static_cast<password_content>(static_cast<std::underlying_type_t<password_content>>(a) ^
		                                     static_cast<std::underlying_type_t<password_content>>(b));
	}

	inline password_content operator~(password_content a) noexcept
	{
		return static_cast<password_content>(~static_cast<std::underlying_type_t<password_content>>(a));
	}

	inline password_content& operator|=(password_content& a, password_content b) noexcept
	{
		return a = a | b;
	}

	inline password_content& operator&=(password_content& a, password_content b) noexcept
	{
		return a = a & b;
	}

	inline password_content& operator^=(password_content& a, password_content b) noexcept
	{
		return a = a ^ b;
	}
}
