#pragma once

#include <cstdint>

namespace desktop::notifications
{
	enum class notification_severity : std::uint8_t
	{
		information,
		success,
		warning,
		error
	};
}