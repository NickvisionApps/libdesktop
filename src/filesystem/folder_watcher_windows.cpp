#include "filesystem/folder_watcher.h"
#include <windows.h>
#include <algorithm>
#include <array>
#include <stdexcept>
#include <thread>
#include <vector>

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

	class folder_watcher::state
	{
	public:
		HANDLE folder{ INVALID_HANDLE_VALUE };
		HANDLE terminate_event{ nullptr };
		std::thread thread;

		static void watcher(folder_watcher* watcher);
	};

	void folder_watcher::state::watcher(folder_watcher* watcher)
	{
		OVERLAPPED overlapped{};
		overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!overlapped.hEvent)
		{
			return;
		}
		std::vector<BYTE> buffer(1024 * 256);
		std::array<HANDLE, 2> wait_handles{ overlapped.hEvent, watcher->m_state->terminate_event };
		while (true)
		{
			ResetEvent(overlapped.hEvent);
			DWORD bytes{ 0 };
			BOOL ok{ ReadDirectoryChangesW(watcher->m_state->folder, buffer.data(), static_cast<DWORD>(buffer.size()), FALSE,
				                           FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
				                               FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS,
				                           nullptr, &overlapped, nullptr) };
			if (!ok)
			{
				break;
			}
			DWORD wait_result{ WaitForMultipleObjects(2, wait_handles.data(), FALSE, INFINITE) };
			if (wait_result == WAIT_OBJECT_0 + 1)
			{
				CancelIoEx(watcher->m_state->folder, &overlapped);
				break;
			}
			if (wait_result != WAIT_OBJECT_0)
			{
				break;
			}
			if (!GetOverlappedResult(watcher->m_state->folder, &overlapped, &bytes, FALSE) || bytes == 0)
			{
				continue;
			}
			BYTE* ptr{ buffer.data() };
			while (true)
			{
				FILE_NOTIFY_INFORMATION* info{ reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr) };
				std::wstring rel{ info->FileName, info->FileNameLength / sizeof(WCHAR) };
				watcher->fire(watcher->m_path / rel, action_to_flag(info->Action));
				if (info->NextEntryOffset == 0)
				{
					break;
				}
				ptr += info->NextEntryOffset;
			}
		}
		CloseHandle(overlapped.hEvent);
	}

	folder_watcher::folder_watcher(std::filesystem::path path)
	    : m_state{ std::make_unique<state>() },
	      m_path{ std::move(path) }
	{
		m_state->folder = CreateFileW(m_path.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
		                              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
		if (m_state->folder == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error("Unable to open folder");
		}
		m_state->terminate_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!m_state->terminate_event)
		{
			CloseHandle(m_state->folder);
			throw std::runtime_error("Unable to create terminate event");
		}
		m_state->thread = std::thread(&state::watcher, this);
	}

	folder_watcher::~folder_watcher()
	{
		std::unique_lock lock{ m_mutex };
		m_stopping = true;
		lock.unlock();
		if (m_state->terminate_event)
		{
			SetEvent(m_state->terminate_event);
		}
		m_cv.notify_all();
		if (m_state->thread.joinable())
		{
			m_state->thread.join();
		}
		if (m_state->terminate_event)
		{
			CloseHandle(m_state->terminate_event);
		}
		if (m_state->folder != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_state->folder);
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
		std::unique_lock lock{ m_mutex };
		m_queue.push_back(flag);
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

	bool folder_watcher::wait_for_change(folder_watcher_change_flag flag) const
	{
		std::unique_lock lock{ m_mutex };
		while (true)
		{
			if (m_stopping)
			{
				return false;
			}
			if (flag == folder_watcher_change_flag::any)
			{
				if (!m_queue.empty())
				{
					m_queue.pop_front();
					return true;
				}
			}
			else
			{
				std::deque<folder_watcher_change_flag>::iterator it{ std::find(m_queue.begin(), m_queue.end(), flag) };
				if (it != m_queue.end())
				{
					m_queue.erase(m_queue.begin(), std::next(it));
					return true;
				}
			}
			m_cv.wait(lock);
		}
	}
}