#pragma once

#include <cstdint>

namespace desktop::system
{
	enum class process_status : std::uint8_t
	{
		created,
		running,
		killed,
		paused,
		completed
	};
}