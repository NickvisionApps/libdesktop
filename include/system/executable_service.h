#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include "app/configuration_service.h"
#include "network/download_progress.h"
#include "process_result.h"
#include "updates/update_service.h"
#include "updates/version.h"

namespace desktop::system
{
	class executable_service
	{
	public:
		executable_service(std::string executable_name, updates::version bundled_version, std::string asset_name,
		                   std::shared_ptr<app::configuration_service> configuration_service, std::shared_ptr<updates::update_service> stable_update_service,
		                   std::shared_ptr<updates::update_service> preview_update_service = nullptr);
		virtual ~executable_service() = default;
		executable_service(const executable_service&) = delete;
		executable_service(executable_service&&) = delete;
		bool download_update(const updates::version& version, const std::function<void(const network::download_progress&)>& progress = {});
		process_result execute(const std::vector<std::string>& arguments);
		const updates::version& get_bundled_version() const;
		virtual std::filesystem::path get_executable_path() const;
		updates::version get_executable_version(const std::string& version_argument = "--version") const;
		const updates::version& get_latest_version(bool preview) const;
		virtual updates::version get_installed_version() const;
		executable_service& operator=(const executable_service&) = delete;
		executable_service& operator=(executable_service&&) = delete;

	protected:
		mutable std::mutex m_mutex;
		mutable std::filesystem::path m_executable_path;
		mutable updates::version m_latest_stable_version;
		mutable updates::version m_latest_preview_version;
		std::string m_executable_name;
		updates::version m_bundled_version;
		std::string m_asset_name;
		std::shared_ptr<app::configuration_service> m_configuration_service;
		std::shared_ptr<updates::update_service> m_update_service;
		std::shared_ptr<updates::update_service> m_preview_update_service;
	};
}