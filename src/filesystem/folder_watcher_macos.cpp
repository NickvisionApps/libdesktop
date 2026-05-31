#include "filesystem/folder_watcher.h"
#include <algorithm>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <span>
#include <stdexcept>

namespace desktop::filesystem
{
	class folder_watcher::state
	{
	public:
		FSEventStreamRef stream{ nullptr };
		dispatch_queue_t queue{ nullptr };

		static void callback(ConstFSEventStreamRef stream, void* client_info, std::size_t num_queue, void* event_paths,
		                     const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId ids[]);
	};

	void folder_watcher::state::callback(ConstFSEventStreamRef stream, void* client_info, std::size_t num_queue, void* event_paths,
	                                     const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId ids[])
	{
		folder_watcher* watcher{ static_cast<folder_watcher*>(client_info) };
		std::span<char*> paths{ static_cast<char**>(event_paths), num_queue };
		for (std::size_t i{ 0 }; i < num_queue; i++)
		{
			std::filesystem::path full_path{ paths[i] };
			if (event_flags[i] & kFSEventStreamEventFlagItemRemoved || !std::filesystem::exists(full_path))
			{
				watcher->fire(full_path, folder_watcher_change_flag::removed);
			}
			if (event_flags[i] & kFSEventStreamEventFlagItemRenamed)
			{
				watcher->fire(full_path, folder_watcher_change_flag::renamed);
			}
			if (event_flags[i] & kFSEventStreamEventFlagItemCreated)
			{
				watcher->fire(full_path, folder_watcher_change_flag::added);
			}
			if (event_flags[i] & kFSEventStreamEventFlagItemModified)
			{
				watcher->fire(full_path, folder_watcher_change_flag::modified);
			}
		}
	}

	folder_watcher::folder_watcher(std::filesystem::path path)
	    : m_state{ std::make_unique<state>() },
	      m_path{ std::move(path) }
	{
		std::string str{ m_path.string() };
		CFStringRef cf_path{ CFStringCreateWithCString(kCFAllocatorDefault, str.c_str(), kCFStringEncodingUTF8) };
		if (!cf_path)
		{
			throw std::runtime_error("Unable to create string");
		}
		CFArrayRef paths{ CFArrayCreate(kCFAllocatorDefault, reinterpret_cast<const void**>(&cf_path), 1, &kCFTypeArrayCallBacks) };
		CFRelease(cf_path);
		if (!paths)
		{
			throw std::runtime_error("Unable to create array");
		}
		FSEventStreamContext context{};
		context.info = this;
		m_state->stream = FSEventStreamCreate(kCFAllocatorDefault, &state::callback, &context, paths, kFSEventStreamEventIdSinceNow, 0.25,
		                                      kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer);
		CFRelease(paths);
		if (!m_state->stream)
		{
			throw std::runtime_error("Unable to initialize filesystem event");
		}
		m_state->queue = dispatch_queue_create("desktop.filesystem.folder_watcher", DISPATCH_QUEUE_SERIAL);
		if (!m_state->queue)
		{
			FSEventStreamInvalidate(m_state->stream);
			FSEventStreamRelease(m_state->stream);
			throw std::runtime_error("Unable to create dispatch queue");
		}
		FSEventStreamSetDispatchQueue(m_state->stream, m_state->queue);
		if (!FSEventStreamStart(m_state->stream))
		{
			FSEventStreamSetDispatchQueue(m_state->stream, nullptr);
			FSEventStreamInvalidate(m_state->stream);
			FSEventStreamRelease(m_state->stream);
#if !OS_OBJECT_USE_OBJC
			dispatch_release(m_state->queue);
#endif
			throw std::runtime_error("Unable to start filesystem event stream");
		}
	}

	folder_watcher::~folder_watcher()
	{
		std::unique_lock lock{ m_mutex };
		m_stopping = true;
		lock.unlock();
		if (m_state->stream)
		{
			FSEventStreamStop(m_state->stream);
			FSEventStreamSetDispatchQueue(m_state->stream, nullptr);
			FSEventStreamInvalidate(m_state->stream);
			FSEventStreamRelease(m_state->stream);
		}
#if !OS_OBJECT_USE_OBJC
		if (m_state->queue)
		{
			dispatch_release(m_state->queue);
		}
#endif
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