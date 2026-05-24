#include "filesystem/folder_watcher.h"
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <sys/inotify.h>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace desktop::filesystem
{
	class folder_watcher::impl
	{
	public:
		impl(folder_watcher& owner)
		    : m_owner{ owner }
		{
			m_inotify_fd = inotify_init1(IN_CLOEXEC);
			if (m_inotify_fd < 0)
			{
				throw std::runtime_error("Unable to initalize watcher");
			}
			constexpr uint32_t MASK{ IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF | IN_MODIFY | IN_ATTRIB |
				                     IN_CLOSE_WRITE };
			m_watch_fd = inotify_add_watch(m_inotify_fd, owner.m_path.c_str(), MASK);
			if (m_watch_fd < 0)
			{
				::close(m_inotify_fd);
				throw std::runtime_error("Unable to initalize watcher");
			}
			int pipe_fds[2];
			if (::pipe(pipe_fds) < 0)
			{
				inotify_rm_watch(m_inotify_fd, m_watch_fd);
				::close(m_inotify_fd);
				throw std::runtime_error("Unable to initalize watcher");
			}
			m_pipe_r = pipe_fds[0];
			m_pipe_w = pipe_fds[1];
			m_thread = std::thread(&impl::watch_loop, this);
		}

		~impl()
		{
			const char byte{ 0 };
			::write(m_pipe_w, &byte, 1);
			m_wait_cv.notify_all();
			if (m_thread.joinable())
			{
				m_thread.join();
			}
			::close(m_pipe_w);
			::close(m_pipe_r);
			inotify_rm_watch(m_inotify_fd, m_watch_fd);
			::close(m_inotify_fd);
		}

		void wait_for_change(folder_watcher_change_flag flag) const
		{
			std::unique_lock<std::mutex> lk{ m_wait_mutex };
			m_wait_cv.wait(lk, [&]
			{
				return m_notified && (flag == folder_watcher_change_flag::any || m_last_flag == flag);
			});
			m_notified = false;
		}

	private:
		void fire(const std::filesystem::path& full_path, folder_watcher_change_flag flag)
		{
			folder_watcher_event_args args{ full_path, flag };
			switch (flag)
			{
			case folder_watcher_change_flag::added:
				m_owner.m_created_event.invoke(m_owner, args);
				break;
			case folder_watcher_change_flag::removed:
				m_owner.m_deleted_event.invoke(m_owner, args);
				break;
			case folder_watcher_change_flag::renamed:
				m_owner.m_renamed_event.invoke(m_owner, args);
				break;
			default:
				break;
			}
			m_owner.m_changed_event.invoke(m_owner, args);
			{
				std::scoped_lock lk{ m_wait_mutex };
				m_last_flag = flag;
				m_notified = true;
			}
			m_wait_cv.notify_all();
		}

		void watch_loop()
		{
			constexpr std::size_t BUF_LEN{ 1024 * (sizeof(inotify_event) + NAME_MAX + 1) };
			std::vector<char> buf(BUF_LEN);
			while (true)
			{
				fd_set read_fds;
				FD_ZERO(&read_fds);
				FD_SET(m_inotify_fd, &read_fds);
				FD_SET(m_pipe_r, &read_fds);
				int nfds{ (m_inotify_fd > m_pipe_r ? m_inotify_fd : m_pipe_r) + 1 };
				if (::select(nfds, &read_fds, nullptr, nullptr, nullptr) < 0)
				{
					break;
				}
				if (FD_ISSET(m_pipe_r, &read_fds))
				{
					break;
				}
				if (!FD_ISSET(m_inotify_fd, &read_fds))
				{
					continue;
				}
				ssize_t length{ ::read(m_inotify_fd, buf.data(), buf.size()) };
				if (length <= 0)
				{
					continue;
				}
				for (ssize_t i{ 0 }; i < length;)
				{
					inotify_event* ev{ reinterpret_cast<inotify_event*>(&buf[i]) };
					i += static_cast<ssize_t>(sizeof(inotify_event)) + ev->len;
					std::filesystem::path full_path{ ev->len > 0 ? m_owner.m_path / ev->name : m_owner.m_path };
					if (ev->mask & (IN_CREATE | IN_MOVED_TO))
					{
						fire(full_path, folder_watcher_change_flag::added);
					}
					else if (ev->mask & (IN_DELETE | IN_DELETE_SELF))
					{
						fire(full_path, folder_watcher_change_flag::removed);
					}
					else if (ev->mask & (IN_MOVED_FROM | IN_MOVE_SELF))
					{
						fire(full_path, folder_watcher_change_flag::renamed);
					}
					else
					{
						fire(full_path, folder_watcher_change_flag::modified);
					}
				}
			}
		}
		folder_watcher& m_owner;
		int m_inotify_fd{ -1 };
		int m_watch_fd{ -1 };
		int m_pipe_r{ -1 };
		int m_pipe_w{ -1 };
		std::thread m_thread;
		mutable std::mutex m_wait_mutex;
		mutable std::condition_variable m_wait_cv;
		mutable folder_watcher_change_flag m_last_flag{ folder_watcher_change_flag::any };
		mutable bool m_notified{ false };
	};

	folder_watcher::folder_watcher(std::filesystem::path path)
	    : m_path{ std::move(path) },
	      m_impl{ std::make_unique<impl>(*this) }
	{
	}

	folder_watcher::~folder_watcher() = default;

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

	void folder_watcher::wait_for_change(folder_watcher_change_flag change_flag) const
	{
		m_impl->wait_for_change(change_flag);
	}
}