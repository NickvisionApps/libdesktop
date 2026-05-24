#pragma once

#include <cstdint>

namespace desktop::filesystem
{
	enum class folder_watcher_change_flag : std::uint8_t
	{
		any,
		added,
		removed,
		modified,
		renamed
	};
}