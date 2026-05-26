#include "hosting/single_instance_running_error.h"

namespace desktop::hosting
{
	single_instance_running_error::single_instance_running_error()
	    : std::runtime_error{ "An instance of this single-instance application is already running" }
	{
	}
}