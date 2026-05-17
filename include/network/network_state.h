#pragma once

#include <cstdint>

namespace desktop::network
{
	enum class network_state : std::uint8_t
	{
		disconnected,
		connected_local,
		connected_global
	};
}