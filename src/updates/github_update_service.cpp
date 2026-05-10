#include "updates/github_update_service.h"
#include <format>
#include <stdexcept>
#include "filesystem/user_directories.h"
#include "helpers/hash.h"
#include "helpers/string_manip.h"

using namespace desktop::app;
using namespace desktop::filesystem;
using namespace desktop::helpers;
using namespace desktop::network;

namespace desktop::updates
{
	github_update_service::github_update_service(const std::shared_ptr<app_info>& info, std::shared_ptr<http_service> http_service)
	    : m_http_service{ std::move(http_service) }
	{
		try
		{
			std::vector<std::string> fields{ string_manip::split(info->get_source_url(), '/') };
			m_owner = fields[3];
			m_repo = fields[4];
		}
		catch (...)
		{
			throw std::invalid_argument{ "Invalid source URL" };
		}
		m_cache_releases_path = user_directories::get_cache() / std::format("{}-{}-releases.json", m_owner, m_repo);
		std::filesystem::create_directories(m_cache_releases_path.parent_path());
	}

	github_update_service::github_update_service(const std::string& owner, const std::string& repo, std::shared_ptr<http_service> http_service)
	    : m_owner{ owner },
	      m_repo{ repo },
	      m_http_service{ std::move(http_service) },
	      m_cache_releases_path{ user_directories::get_cache() / std::format("{}-{}-releases.json", owner, repo) }
	{
		std::filesystem::create_directories(m_cache_releases_path.parent_path());
	}

	bool github_update_service::download_asset(const version& version, std::string_view name, const std::filesystem::path& destination, bool exact_match,
	                                           const std::function<void(const download_progress&)>& progress)
	{
		return false;
	}

	version github_update_service::get_latest_preview_version() const
	{
		return {};
	}

	version github_update_service::get_latest_stable_version() const
	{
		return {};
	}

#ifdef _WIN32
	bool github_update_service::install_windows_update(const version& version, const std::function<void(const network::download_progress&)>& progress)
	{
		return false;
	}
#endif
}