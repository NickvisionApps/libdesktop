#pragma once

#include <filesystem>
#include <functional>
#include <string_view>
#include "network/download_progress.h"
#include "version.h"

namespace desktop::updates
{
	class update_service
	{
	public:
		update_service() = default;
		virtual ~update_service() = default;
		update_service(const update_service&) = delete;
		update_service(update_service&&) = delete;
		virtual bool download_asset(const version& version, std::string_view name, const std::filesystem::path& destination, bool exact_match = true,
		                            const std::function<void(const network::download_progress&)>& progress = {}) = 0;
		virtual version get_latest_preview_version() const = 0;
		virtual version get_latest_stable_version() const = 0;
#ifdef _WIN32
		virtual bool install_windows_update(const version& version, const std::function<void(const network::download_progress&)>& progress = {}) = 0;
#endif
		update_service& operator=(const update_service&) = delete;
		update_service& operator=(update_service&&) = delete;
	};
}