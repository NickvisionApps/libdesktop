#include "hosting/single_instance_mutex.h"
#include <fcntl.h>
#include <mutex>
#include <set>
#include <string>
#include <unistd.h>

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
		int m_fd{ -1 };
	};

	single_instance_mutex::impl::impl(single_instance_mutex& mutex)
	    : m_mutex{ mutex }
	{
	}

	single_instance_mutex::impl::~impl()
	{
		unlock();
	}

	bool single_instance_mutex::impl::is_locked() const
	{
		return m_fd != -1;
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
		std::string path{ "/tmp/" + m_mutex.m_name + ".lock" };
		int fd{ open(path.c_str(), O_CREAT | O_RDWR, 0666) };
		if (fd < 0)
		{
			std::scoped_lock reg{ s_registry_mutex };
			s_locked_names.erase(m_mutex.m_name);
			return false;
		}
		struct flock fl{};
		fl.l_type = F_WRLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;
		if (fcntl(fd, F_SETLK, &fl) < 0)
		{
			close(fd);
			std::scoped_lock reg{ s_registry_mutex };
			s_locked_names.erase(m_mutex.m_name);
			return false;
		}
		std::string pid{ std::to_string(getpid()) };
		ftruncate(fd, 0);
		write(fd, pid.c_str(), pid.size());
		m_fd = fd;
		return true;
	}

	void single_instance_mutex::impl::unlock()
	{
		if (m_fd == -1)
		{
			return;
		}
		struct flock fl{};
		fl.l_type = F_UNLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;
		fcntl(m_fd, F_SETLK, &fl);
		close(m_fd);
		m_fd = -1;
		std::scoped_lock reg{ s_registry_mutex };
		s_locked_names.erase(m_mutex.m_name);
	}

	single_instance_mutex::single_instance_mutex(std::string name)
	    : m_impl{ std::make_unique<impl>(*this) },
	      m_name{ std::move(name) }
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