#pragma once

#include <mutex>
#ifdef __APPLE__
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

namespace desktop::system
{
	class power_service
	{
	public:
		power_service() = default;
		~power_service();
		power_service(const power_service&) = delete;
		power_service(power_service&&) = delete;
		bool is_suspended() const;
		bool allow_suspend();
		bool prevent_suspend();
		power_service& operator=(const power_service&) = delete;
		power_service& operator=(power_service&&) = delete;

	private:
		mutable std::mutex m_mutex;
		bool m_suspended{ false };
#ifdef __linux__
		unsigned int m_cookie{ 0 };
#elif defined(__APPLE__)
		IOPMAssertionID m_cookie{ kIOPMNullAssertionID };
#endif
	};
}