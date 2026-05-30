#include "filesystem/folder_watcher.h"
#include <climits>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <sys/inotify.h>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace desktop::filesystem
{
	class folder_watcher::state
	{
	public:
		int inotify_fd{ -1 };
		int watch_fd{ -1 };
		int pipe_r{ -1 };
		int pipe_w{ -1 };
		std::thread thread;

		static void watcher(folder_watcher* watcher);
	};

	void folder_watcher::state::watcher(folder_watcher* watcher)
	{
		constexpr std::size_t BUF_LEN{ 1024 * (sizeof(inotify_event) + NAME_MAX + 1) };
		std::vector<char> buf(BUF_LEN);
		while (true)
		{
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(watcher->m_state->inotify_fd, &read_fds);
			FD_SET(watcher->m_state->pipe_r, &read_fds);
			int nfds{ (watcher->m_state->inotify_fd > watcher->m_state->pipe_r ? watcher->m_state->inotify_fd : watcher->m_state->pipe_r) + 1 };
			if (::select(nfds, &read_fds, nullptr, nullptr, nullptr) < 0)
			{
				break;
			}
			if (FD_ISSET(watcher->m_state->pipe_r, &read_fds))
			{
				break;
			}
			if (!FD_ISSET(watcher->m_state->inotify_fd, &read_fds))
			{
				continue;
			}
			ssize_t length{ ::read(watcher->m_state->inotify_fd, buf.data(), buf.size()) };
			if (length <= 0)
			{
				continue;
			}
			for (ssize_t i{ 0 }; i < length;)
			{
				inotify_event* ev{ reinterpret_cast<inotify_event*>(&buf[i]) };
				i += static_cast<ssize_t>(sizeof(inotify_event)) + ev->len;
				std::filesystem::path full_path{ ev->len > 0 ? watcher->m_path / ev->name : watcher->m_path };
				if (ev->mask & (IN_MOVED_FROM | IN_MOVE_SELF))
				{
					watcher->fire(full_path, folder_watcher_change_flag::renamed);
				}
				else if (ev->mask & (IN_DELETE | IN_DELETE_SELF))
				{
					watcher->fire(full_path, folder_watcher_change_flag::removed);
				}
				else if (ev->mask & (IN_CREATE | IN_MOVED_TO))
				{
					watcher->fire(full_path, folder_watcher_change_flag::added);
				}
				else
				{
					watcher->fire(full_path, folder_watcher_change_flag::modified);
				}
			}
		}
	}

	folder_watcher::folder_watcher(std::filesystem::path path)
	    : m_state{ std::make_unique<state>() },
	      m_path{ std::move(path) }
	{
		m_state->inotify_fd = inotify_init1(IN_CLOEXEC);
		if (m_state->inotify_fd < 0)
		{
			throw std::runtime_error("Unable to initialize watcher");
		}
		constexpr uint32_t MASK{ IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF | IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE };
		m_state->watch_fd = inotify_add_watch(m_state->inotify_fd, m_path.c_str(), MASK);
		if (m_state->watch_fd < 0)
		{
			::close(m_state->inotify_fd);
			throw std::runtime_error("Unable to initialize watcher");
		}
		int pipe_fds[2];
		if (::pipe(pipe_fds) < 0)
		{
			inotify_rm_watch(m_state->inotify_fd, m_state->watch_fd);
			::close(m_state->inotify_fd);
			throw std::runtime_error("Unable to initialize watcher");
		}
		m_state->pipe_r = pipe_fds[0];
		m_state->pipe_w = pipe_fds[1];
		m_state->thread = std::thread(&state::watcher, this);
	}

	folder_watcher::~folder_watcher()
	{
		if (m_state->pipe_w >= 0)
		{
			const char byte{ 0 };
			::write(m_state->pipe_w, &byte, 1);
		}
		m_cv.notify_all();
		if (m_state->thread.joinable())
		{
			m_state->thread.join();
		}
		if (m_state->pipe_w >= 0)
		{
			::close(m_state->pipe_w);
		}
		if (m_state->pipe_r >= 0)
		{
			::close(m_state->pipe_r);
		}
		if (m_state->watch_fd >= 0)
		{
			inotify_rm_watch(m_state->inotify_fd, m_state->watch_fd);
		}
		if (m_state->inotify_fd >= 0)
		{
			::close(m_state->inotify_fd);
		}
	}

	const std::filesystem::path& folder_watcher::get_path() const
	{
		return m_path;
	}

	const events::event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_changed_event() const
	{
		return m_changed_event;
	}

	const events::event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_created_event() const
	{
		return m_created_event;
	}

	const events::event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_deleted_event() const
	{
		return m_deleted_event;
	}

	const events::event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_renamed_event() const
	{
		return m_renamed_event;
	}

	void folder_watcher::fire(const std::filesystem::path& full_path, folder_watcher_change_flag flag)
	{
		std::unique_lock<std::mutex> lock{ m_mutex };
		m_last_flag = flag;
		lock.unlock();
		m_cv.notify_all();
		folder_watcher_event_args args{ full_path, flag };
		switch (flag)
		{
		case folder_watcher_change_flag::added:
			m_created_event.invoke(*this, args);
			return;
		case folder_watcher_change_flag::removed:
			m_deleted_event.invoke(*this, args);
			return;
		case folder_watcher_change_flag::renamed:
			m_renamed_event.invoke(*this, args);
			return;
		default:
			m_changed_event.invoke(*this, args);
			return;
		}
	}

	void folder_watcher::wait_for_change(folder_watcher_change_flag flag) const
	{
		std::unique_lock lock{ m_mutex };
		m_cv.wait(lock, [this, flag]()
		{
			if (!m_last_flag.has_value())
			{
				return false;
			}
			return flag == folder_watcher_change_flag::any || flag == *m_last_flag;
		});
		m_last_flag = std::nullopt;
	}
}