#include "hosting/single_instance_mutex.h"
#include <windows.h>

namespace desktop::hosting
{
	class single_instance_mutex::state
	{
	public:
		HANDLE handle{ nullptr };
	};

	std::mutex single_instance_mutex::s_registry_mutex{};
	std::set<std::string> single_instance_mutex::s_locked_names{}; // NOLINT(bugprone-throwing-static-initialization)

	single_instance_mutex::single_instance_mutex(std::string name)
	    : m_state{ std::make_unique<state>() },
	      m_name{ std::move(name) }
	{
	}

	single_instance_mutex::~single_instance_mutex()
	{
		unlock();
	}

	bool single_instance_mutex::is_locked() const
	{
		return m_state->handle != nullptr;
	}

	bool single_instance_mutex::lock()
	{
		{
			std::scoped_lock reg{ s_registry_mutex };
			if (!s_locked_names.insert(m_name).second)
			{
				return false;
			}
		}
		m_state->handle = CreateMutexA(nullptr, TRUE, m_name.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			if (m_state->handle != nullptr)
			{
				CloseHandle(m_state->handle);
				m_state->handle = nullptr;
			}
			std::scoped_lock reg{ s_registry_mutex };
			s_locked_names.erase(m_name);
			return false;
		}
		return true;
	}

	void single_instance_mutex::unlock()
	{
		if (m_state->handle == nullptr)
		{
			return;
		}
		ReleaseMutex(m_state->handle);
		CloseHandle(m_state->handle);
		m_state->handle = nullptr;
		std::scoped_lock reg{ s_registry_mutex };
		s_locked_names.erase(m_name);
	}
}