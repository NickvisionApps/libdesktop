#pragma once

#include <filesystem>
#include <string>
#include "events/event_args.h"
#include "folder_watcher_change_flag.h"

namespace desktop::filesystem
{
	class folder_watcher_event_args : public events::event_args
	{
	public:
		folder_watcher_event_args(std::filesystem::path full_path, folder_watcher_change_flag change_flag);
		const std::filesystem::path& get_full_path() const;
		const std::string& get_name() const;
		folder_watcher_change_flag get_change_flag() const;

	private:
		std::filesystem::path m_full_path;
		std::string m_name;
		folder_watcher_change_flag m_change_flag;
	};
}