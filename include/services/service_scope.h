#pragma once

#include <cstdint>

namespace desktop::services
{
	enum class service_scope : std::uint8_t
	{
		singleton,
		transient,
		latched
	};
}