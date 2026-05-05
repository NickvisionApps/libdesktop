#pragma once

namespace desktop::system
{
	enum class process_status
	{
		created,
		running,
		killed,
		paused,
		completed
	};
}