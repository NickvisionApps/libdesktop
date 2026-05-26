#include "hosting/single_instance_mutex.h"
#include <windows.h>
#include <mutex>
#include <set>
#include <string>

static std::mutex s_registry_mutex{};

static std::set<std::string> s_locked_names{};

namespace desktop::hosting
{
	class single_instance_mutex::impl
	{
	public:
		impl(single_instance_mutex& mutex);
		~impl();
		impl(const impl&) = delete;
		impl(impl&&) = delete;
		impl& operator=(const impl&) = delete;
		impl& operator=(impl&&) = delete;
		bool is_locked() const;
		bool lock();
		void unlock();

	private:
		single_instance_mutex& m_mutex;
		HANDLE m_handle;
	};

	single_instance_mutex::impl::impl(single_instance_mutex& mutex)
	    : m_mutex{ mutex },
	      m_handle{ nullptr }
	{
	}

	single_instance_mutex::impl::~impl()
	{
		unlock();
	}

	bool single_instance_mutex::impl::is_locked() const
	{
		return m_handle != nullptr;
	}

	bool single_instance_mutex::impl::lock()
	{
		{
			std::scoped_lock reg{ s_registry_mutex };
			if (!s_locked_names.insert(m_mutex.m_name).second)
			{
				return false;
			}
		}
		m_handle = CreateMutexA(nullptr, TRUE, m_mutex.m_name.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(m_handle);
			m_handle = nullptr;
			std::scoped_lock reg{ s_registry_mutex };
			s_locked_names.erase(m_mutex.m_name);
			return false;
		}
		return true;
	}

	void single_instance_mutex::impl::unlock()
	{
		if (m_handle == nullptr)
		{
			return;
		}
		ReleaseMutex(m_handle);
		CloseHandle(m_handle);
		m_handle = nullptr;
		std::scoped_lock reg{ s_registry_mutex };
		s_locked_names.erase(m_mutex.m_name);
	}

	single_instance_mutex::single_instance_mutex(std::string name)
	    : m_name{ std::move(name) },
	      m_impl{ std::make_unique<impl>(*this) }
	{
	}

	single_instance_mutex::~single_instance_mutex() = default;

	bool single_instance_mutex::is_locked() const
	{
		return m_impl->is_locked();
	}

	bool single_instance_mutex::lock()
	{
		return m_impl->lock();
	}

	void single_instance_mutex::unlock()
	{
		m_impl->unlock();
	}
}