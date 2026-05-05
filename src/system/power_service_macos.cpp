#include "system/power_service.h"

namespace desktop::system
{
	power_service::power_service()
	    : m_suspended{ false },
	      m_cookie{ kIOPMNullAssertionID }
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
		if (IOPMAssertionRelease(m_cookie) != kIOReturnSuccess)
		{
			return false;
		}
		m_cookie = kIOPMNullAssertionID;
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
		if (IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleSystemSleep, kIOPMAssertionLevelOn, CFSTR("Preventing suspend"), &m_cookie) !=
		    kIOReturnSuccess)
		{
			return false;
		}
		m_suspended = true;
		return true;
	}
}
