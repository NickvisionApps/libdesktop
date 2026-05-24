#pragma once

#include <filesystem>
#include <memory>
#include "events/event.h"
#include "folder_watcher_change_flag.h"
#include "folder_watcher_event_args.h"

namespace desktop::filesystem
{
	class folder_watcher
	{
	public:
		folder_watcher(std::filesystem::path path);
		~folder_watcher();
		folder_watcher(const folder_watcher&) = delete;
		folder_watcher(folder_watcher&&) = delete;
		const std::filesystem::path& get_path() const;
		const events::event<folder_watcher, folder_watcher_event_args>& get_changed_event() const;
		const events::event<folder_watcher, folder_watcher_event_args>& get_created_event() const;
		const events::event<folder_watcher, folder_watcher_event_args>& get_deleted_event() const;
		const events::event<folder_watcher, folder_watcher_event_args>& get_renamed_event() const;
		void wait_for_change(folder_watcher_change_flag change_flag) const;
		folder_watcher& operator=(const folder_watcher&) = delete;
		folder_watcher& operator=(folder_watcher&&) = delete;

	private:
		class impl;
		friend class impl;
		std::filesystem::path m_path;
		std::unique_ptr<impl> m_impl;
		events::event<folder_watcher, folder_watcher_event_args> m_changed_event;
		events::event<folder_watcher, folder_watcher_event_args> m_created_event;
		events::event<folder_watcher, folder_watcher_event_args> m_deleted_event;
		events::event<folder_watcher, folder_watcher_event_args> m_renamed_event;
	};
}
