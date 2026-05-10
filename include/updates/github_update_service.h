#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include "app/app_info.h"
#include "network/download_progress.h"
#include "network/http_service.h"
#include "update_service.h"
#include "version.h"

namespace desktop::updates
{
	class github_update_service : public update_service
	{
	public:
		using dependencies = std::tuple<app::app_info, network::http_service>;
		github_update_service(const std::shared_ptr<app::app_info>& info, std::shared_ptr<network::http_service> http_service);
		github_update_service(const std::string& owner, const std::string& repo, std::shared_ptr<network::http_service> http_service);
		~github_update_service() override = default;
		github_update_service(const github_update_service&) = delete;
		github_update_service(github_update_service&&) = delete;
		bool download_asset(const version& version, std::string_view name, const std::filesystem::path& destination, bool exact_match = true,
		                    const std::function<void(const network::download_progress&)>& progress = {}) override;
		version get_latest_preview_version() const override;
		version get_latest_stable_version() const override;
#ifdef _WIN32
		bool install_windows_update(const version& version, const std::function<void(const network::download_progress&)>& progress = {}) override;
#endif
		github_update_service& operator=(const github_update_service&) = delete;
		github_update_service& operator=(github_update_service&&) = delete;

	private:
		std::shared_ptr<network::http_service> m_http_service;
		std::string m_owner;
		std::string m_repo;
		std::filesystem::path m_cache_releases_path;
	};
}