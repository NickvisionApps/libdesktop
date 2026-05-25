#include "filesystem/folder_watcher.h"
#include <windows.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace desktop::events;

namespace desktop::filesystem
{
	static folder_watcher_change_flag action_to_flag(DWORD action) noexcept
	{
		switch (action)
		{
		case FILE_ACTION_ADDED:
			return folder_watcher_change_flag::added;
		case FILE_ACTION_REMOVED:
			return folder_watcher_change_flag::removed;
		case FILE_ACTION_RENAMED_OLD_NAME:
			return folder_watcher_change_flag::renamed;
		case FILE_ACTION_RENAMED_NEW_NAME:
			return folder_watcher_change_flag::renamed;
		default:
			return folder_watcher_change_flag::modified;
		}
	}

	class folder_watcher::impl
	{
	public:
		impl(folder_watcher& owner)
		    : m_owner{ owner }
		{
			m_terminate_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
			if (!m_terminate_event)
			{
				throw std::runtime_error("Unable to create watcher event");
			}
			m_thread = std::thread(&impl::watch_loop, this);
		}

		~impl()
		{
			SetEvent(m_terminate_event);
			m_wait_cv.notify_all();
			if (m_thread.joinable())
			{
				m_thread.join();
			}
			CloseHandle(m_terminate_event);
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
			HANDLE folder{ CreateFileW(m_owner.m_path.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
				                       OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr) };
			if (folder == INVALID_HANDLE_VALUE)
			{
				return;
			}
			OVERLAPPED overlapped{};
			overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
			if (!overlapped.hEvent)
			{
				CloseHandle(folder);
				return;
			}
			std::vector<BYTE> buffer(1024 * 256);
			DWORD bytes{ 0 };
			bool pending{ false };
			HANDLE wait_handles[2]{ overlapped.hEvent, m_terminate_event };
			while (true)
			{
				pending = ReadDirectoryChangesW(folder, buffer.data(), static_cast<DWORD>(buffer.size()), FALSE,
				                                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
				                                    FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS,
				                                &bytes, &overlapped, nullptr);
				if (WaitForMultipleObjects(2, wait_handles, FALSE, INFINITE) != WAIT_OBJECT_0)
				{
					break;
				}
				if (!GetOverlappedResult(folder, &overlapped, &bytes, TRUE) || bytes == 0)
				{
					break;
				}
				pending = false;
				ResetEvent(overlapped.hEvent);
				FILE_NOTIFY_INFORMATION* info{ reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data()) };
				while (true)
				{
					std::wstring rel{ info->FileName, info->FileNameLength / sizeof(WCHAR) };
					fire(m_owner.m_path / rel, action_to_flag(info->Action));
					if (info->NextEntryOffset == 0)
					{
						break;
					}
					info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
				}
			}
			if (pending)
			{
				CancelIo(folder);
				GetOverlappedResult(folder, &overlapped, &bytes, TRUE);
			}
			CloseHandle(overlapped.hEvent);
			CloseHandle(folder);
		}

		folder_watcher& m_owner;
		HANDLE m_terminate_event{ nullptr };
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

	const event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_changed_event() const
	{
		return m_changed_event;
	}

	const event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_created_event() const
	{
		return m_created_event;
	}

	const event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_deleted_event() const
	{
		return m_deleted_event;
	}

	const event<folder_watcher, folder_watcher_event_args>& folder_watcher::get_renamed_event() const
	{
		return m_renamed_event;
	}

	void folder_watcher::wait_for_change(folder_watcher_change_flag change_flag) const
	{
		m_impl->wait_for_change(change_flag);
	}
}