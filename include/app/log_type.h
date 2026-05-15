#pragma once

#include <cstdint>

namespace desktop::app
{
	enum class log_type : std::uint8_t
	{
		debug,
		info,
		warn,
		error,
		critical
	};
}