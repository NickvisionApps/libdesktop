#pragma once

#include <filesystem>
#include <functional>
#include <optional>
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
		virtual bool download_asset(const version& target, std::string name, const std::filesystem::path& destination, bool exact_match = true,
		                            const std::function<void(const network::download_progress&)>& progress = {}) = 0;
		virtual std::optional<version> get_latest_version(bool preview) const = 0;
#ifdef _WIN32
		virtual bool install_update_for_windows(const version& version, const std::function<void(const network::download_progress&)>& progress = {}) = 0;
#endif
		update_service& operator=(const update_service&) = delete;
		update_service& operator=(update_service&&) = delete;
	};
}