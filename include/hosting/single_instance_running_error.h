#pragma once

#include <stdexcept>

namespace desktop::hosting
{
	class single_instance_running_error : public std::runtime_error
	{
	public:
		single_instance_running_error();
	};
}