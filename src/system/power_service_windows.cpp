#include "system/power_service.h"
#include <windows.h>

namespace desktop::system
{
	class power_service::state
	{
	};

	power_service::power_service() = default;

	power_service::~power_service()
	{
		allow_suspend();
	}

	bool power_service::is_suspended() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_suspended;
	}

	bool power_service::allow_suspend()
	{
		std::scoped_lock lock{ m_mutex };
		if (!m_suspended)
		{
			return true;
		}
		if (SetThreadExecutionState(ES_CONTINUOUS) == NULL)
		{
			return false;
		}
		m_suspended = false;
		return true;
	}

	bool power_service::prevent_suspend()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_suspended)
		{
			return true;
		}
		if (SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED) == NULL)
		{
			return false;
		}
		m_suspended = true;
		return true;
	}
}
