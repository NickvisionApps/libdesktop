#include "filesystem/folder_watcher.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <condition_variable>
#include <mutex>
#include <span>
#include <stdexcept>

namespace desktop::filesystem
{
	class folder_watcher::impl
	{
	public:
		impl(folder_watcher& owner);
		~impl();
		impl(const impl&) = delete;
		impl(impl&&) = delete;
		impl& operator=(const impl&) = delete;
		impl& operator=(impl&&) = delete;
		void wait_for_change(folder_watcher_change_flag flag) const;

	private:
		static void fs_callback(ConstFSEventStreamRef, void* client_info, std::size_t num_events, void* event_paths,
		                        const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId[]) noexcept;
		void fire(const std::filesystem::path& full_path, folder_watcher_change_flag flag);
		folder_watcher& m_owner;
		FSEventStreamRef m_stream{ nullptr };
		dispatch_queue_t m_queue{ nullptr };
		mutable std::mutex m_wait_mutex;
		mutable std::condition_variable m_wait_cv;
		mutable folder_watcher_change_flag m_last_flag{ folder_watcher_change_flag::any };
		mutable bool m_notified{ false };
	};

	folder_watcher::impl::impl(folder_watcher& owner)
	    : m_owner{ owner }
	{
		std::string path_str{ owner.m_path.string() };
		CFStringRef cf_path{ CFStringCreateWithCString(kCFAllocatorDefault, path_str.c_str(), kCFStringEncodingUTF8) };
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
		m_stream = FSEventStreamCreate(kCFAllocatorDefault, &impl::fs_callback, &context, paths, kFSEventStreamEventIdSinceNow, 0.25,
		                               kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer);
		CFRelease(paths);
		if (!m_stream)
		{
			throw std::runtime_error("Unable to initialize filesystem event");
		}
		m_queue = dispatch_queue_create("desktop.filesystem.folder_watcher", DISPATCH_QUEUE_SERIAL);
		FSEventStreamSetDispatchQueue(m_stream, m_queue);
		FSEventStreamStart(m_stream);
	}

	folder_watcher::impl::~impl()
	{
		FSEventStreamStop(m_stream);
		FSEventStreamSetDispatchQueue(m_stream, nullptr);
		FSEventStreamInvalidate(m_stream);
		FSEventStreamRelease(m_stream);
		if (m_queue)
		{
			dispatch_release(m_queue);
		}
	}

	void folder_watcher::impl::wait_for_change(folder_watcher_change_flag flag) const
	{
		std::unique_lock<std::mutex> lk{ m_wait_mutex };
		m_wait_cv.wait(lk, [&]
		{
			return m_notified && (flag == folder_watcher_change_flag::any || m_last_flag == flag);
		});
		m_notified = false;
	}

	void folder_watcher::impl::fs_callback(ConstFSEventStreamRef, void* client_info, std::size_t num_events, void* event_paths,
	                                       const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId[]) noexcept
	{
		impl* self{ static_cast<impl*>(client_info) };
		std::span<char*> paths{ std::span<char*>{ static_cast<char**>(event_paths), num_events } };
		std::span<const FSEventStreamEventFlags> flags{ std::span<const FSEventStreamEventFlags>{ event_flags, num_events } };
		for (std::size_t i{ 0 }; i < num_events; ++i)
		{
			std::filesystem::path full_path{ paths[i] };
			bool exists{ std::filesystem::exists(full_path) };
			folder_watcher_change_flag flag{ folder_watcher_change_flag::modified };
			if (!exists)
			{
				flag = folder_watcher_change_flag::removed;
			}
			else if (flags[i] & kFSEventStreamEventFlagItemCreated)
			{
				flag = folder_watcher_change_flag::added;
			}
			else if (flags[i] & kFSEventStreamEventFlagItemRenamed)
			{
				flag = folder_watcher_change_flag::renamed;
			}
			self->fire(full_path, flag);
		}
	}

	void folder_watcher::impl::fire(const std::filesystem::path& full_path, folder_watcher_change_flag flag)
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