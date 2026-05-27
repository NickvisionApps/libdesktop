#include "system/power_service.h"
#include <IOKit/pwr_mgt/IOPMLib.h>

namespace desktop::system
{
	class power_service::state
	{
	public:
		IOPMAssertionID cookie{ kIOPMNullAssertionID };
	};

	power_service::power_service()
	    : m_state{ std::make_unique<state>() }
	{
	}

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
		if (IOPMAssertionRelease(m_state->cookie) != kIOReturnSuccess)
		{
			return false;
		}
		m_state->cookie = kIOPMNullAssertionID;
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
		if (IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleSystemSleep, kIOPMAssertionLevelOn, CFSTR("Preventing suspend"), &m_state->cookie) !=
		    kIOReturnSuccess)
		{
			return false;
		}
		m_suspended = true;
		return true;
	}
}
