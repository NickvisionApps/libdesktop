#pragma once

#include <mutex>
#include "services/service.h"
#ifdef __APPLE__
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

namespace desktop::system
{
	class power_service : public services::service
	{
	public:
		power_service();
		~power_service() override = default;
		power_service(const power_service&) = delete;
		power_service(power_service&&) = delete;
		bool is_suspended() const;
		bool allow_suspend() const;
		bool prevent_suspend();
		power_service& operator=(const power_service&) = delete;
		power_service& operator=(power_service&&) = delete;

	private:
		mutable std::mutex m_mutex;
		bool m_suspended;
#ifdef __linux__
		unsigned int m_cookie;
#elif defined(__APPLE__)
		IOPMAssertionID m_cookie;
#endif
	};
}