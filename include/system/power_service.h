#pragma once

#include <mutex>

namespace desktop::system
{
	class power_service
	{
	public:
		power_service();
		~power_service();
		power_service(const power_service&) = delete;
		power_service(power_service&&) = delete;
		bool is_suspended() const;
		bool allow_suspend();
		bool prevent_suspend();
		power_service& operator=(const power_service&) = delete;
		power_service& operator=(power_service&&) = delete;

	private:
		class state;
		friend class state;
		mutable std::mutex m_mutex;
		std::unique_ptr<state> m_state{ nullptr };
		bool m_suspended{ false };
	};
}