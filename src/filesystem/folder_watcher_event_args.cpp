#include "filesystem/folder_watcher_event_args.h"

namespace desktop::filesystem
{
	folder_watcher_event_args::folder_watcher_event_args(std::filesystem::path full_path, folder_watcher_change_flag change_flag)
	    : m_full_path{ std::move(full_path) },
	      m_name{ full_path.filename().string() },
	      m_change_flag{ change_flag }
	{
	}

	const std::filesystem::path& folder_watcher_event_args::get_full_path() const
	{
		return m_full_path;
	}

	const std::string& folder_watcher_event_args::get_name() const
	{
		return m_name;
	}

	folder_watcher_change_flag folder_watcher_event_args::get_change_flag() const
	{
		return m_change_flag;
	}
}