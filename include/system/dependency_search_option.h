#pragma once

#include <cstdint>

namespace desktop::system
{
	enum class dependency_search_option : std::uint8_t
	{
		global,
		app,
		system,
		local
	};
}