#include "hosting/single_instance_mutex.h"
#include <fcntl.h>
#include <unistd.h>

namespace desktop::hosting
{
	class single_instance_mutex::state
	{
	public:
		int fd{ -1 };
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
		return m_state->fd != -1;
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
		std::string path{ "/tmp/" + m_name + ".lock" };
		int fd{ open(path.c_str(), O_CREAT | O_RDWR, 0666) };
		if (fd < 0)
		{
			std::scoped_lock reg{ s_registry_mutex };
			s_locked_names.erase(m_name);
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
			s_locked_names.erase(m_name);
			return false;
		}
		std::string pid{ std::to_string(getpid()) };
		ftruncate(fd, 0);
		write(fd, pid.c_str(), pid.size());
		m_state->fd = fd;
		return true;
	}

	void single_instance_mutex::unlock()
	{
		if (m_state->fd == -1)
		{
			return;
		}
		struct flock fl{};
		fl.l_type = F_UNLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 0;
		fcntl(m_state->fd, F_SETLK, &fl);
		close(m_state->fd);
		m_state->fd = -1;
		std::scoped_lock reg{ s_registry_mutex };
		s_locked_names.erase(m_name);
	}
}