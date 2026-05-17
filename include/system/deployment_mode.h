#pragma once

#include <cstdint>

namespace desktop::system
{
	enum class deployment_mode : std::uint8_t
	{
		local,
		flatpak,
		snap,
		wsl
	};
}