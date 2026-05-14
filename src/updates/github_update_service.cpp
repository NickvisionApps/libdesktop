#include "updates/github_update_service.h"
#include <chrono>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "filesystem/user_directories.h"
#include "helpers/hash.h"
#include "helpers/string_manip.h"
#ifdef _WIN32
#include <windows.h>
#endif

using namespace desktop::app;
using namespace desktop::filesystem;
using namespace desktop::helpers;
using namespace desktop::network;

namespace desktop::updates
{
	github_update_service::github_update_service(const std::shared_ptr<app_info>& info, std::shared_ptr<http_service> http_service)
	    : m_http_service{ std::move(http_service) }
	{
		std::vector<std::string> fields{ string_manip::split(info->get_source_url(), '/') };
		if (fields.size() < 5)
		{
			throw std::invalid_argument{ "Invalid source URL" };
		}
		m_owner = fields[3];
		m_repo = fields[4];
		m_cache_releases_path = user_directories::get_cache() / std::format("{}-{}-releases.json", m_owner, m_repo);
		std::filesystem::create_directories(m_cache_releases_path.parent_path());
	}

	github_update_service::github_update_service(const std::string& owner, const std::string& repo, std::shared_ptr<http_service> http_service)
	    : m_http_service{ std::move(http_service) },
          m_owner{ owner },
	      m_repo{ repo },
	      m_cache_releases_path{ user_directories::get_cache() / std::format("{}-{}-releases.json", owner, repo) }
	{
		std::filesystem::create_directories(m_cache_releases_path.parent_path());
	}

	bool github_update_service::download_asset(const version& target, std::string name, const std::filesystem::path& destination, bool exact_match,
	                                           const std::function<void(const download_progress&)>& progress)
	{
		name = string_manip::lower(name);
		for (const github_release& release : get_all_releases())
		{
			std::optional<version> v{ version::parse(string_manip::trim(release.get_tag_name(), 'v')) };
			if (!v.has_value() || v != target)
			{
				continue;
			}
			for (const github_release_asset& asset : release.get_assets())
			{
				std::string lower_asset_name{ string_manip::lower(asset.get_name()) };
				if (exact_match ? lower_asset_name != name : lower_asset_name.find(name) == std::string::npos)
				{
					continue;
				}
				if (m_http_service->download_file(asset.get_browser_download_url(), destination, true, progress))
				{
					if (string_manip::replace_all(asset.get_digest(), "sha256:", "") == hash::sha256(destination))
					{
						return true;
					}
				}
				try
				{
					std::filesystem::remove(destination);
				}
				catch (...)
				{
				}
			}
		}
		return false;
	}

	std::optional<version> github_update_service::get_latest_version(bool preview) const
	{
		for (const github_release& release : get_all_releases())
		{
			if (!release.get_tag_name().empty() && release.is_prerelease() == preview && !release.is_draft())
			{
				std::optional<version> version{ version::parse(release.get_tag_name()) };
				if (version.has_value())
				{
					return *version;
				}
			}
		}
		return std::nullopt;
	}

#ifdef _WIN32
	bool github_update_service::install_update_for_windows(const version& version, const std::function<void(const network::download_progress&)>& progress)
	{
		std::filesystem::path setup{ user_directories::get_cache() / std::format("{}_{}_Setup.exe", m_owner, m_repo) };
#ifdef _M_ARM64
		if (!download_asset(version, "setup-arm64.exe", setup, false, progress))
		{
			if (!download_asset(version, "setup.exe", setup, false, progress))
			{
				return false;
			}
		}
#else
		if (!download_asset(version, "setup-x64.exe", setup, false, progress))
		{
			if (!download_asset(version, "setup.exe", setup, false, progress))
			{
				return false;
			}
		}
#endif
		SHELLEXECUTEINFOW info{
			.cbSize = sizeof(SHELLEXECUTEINFOW),
			.lpVerb = L"runas",
			.lpFile = setup.c_str(),
			.nShow = SW_SHOWNORMAL,
		};
		return ShellExecuteExW(&info) == TRUE;
	}
#endif

	const std::vector<github_release>& github_update_service::get_all_releases() const
	{
		if (!m_releases.empty())
		{
			return m_releases;
		}
		if (std::filesystem::exists(m_cache_releases_path))
		{
			std::chrono::time_point<std::chrono::file_clock> time{ std::filesystem::last_write_time(m_cache_releases_path) };
			if (std::chrono::duration_cast<std::chrono::hours>(std::chrono::file_clock::now() - time).count() > 6)
			{
				std::filesystem::remove(m_cache_releases_path);
			}
		}
		m_releases.clear();
		if (std::filesystem::exists(m_cache_releases_path))
		{
			std::ifstream file{ m_cache_releases_path };
			nlohmann::json json;
			file >> json;
			for (const nlohmann::json& release : json)
			{
				m_releases.emplace_back(release);
			}
		}
		if (m_releases.empty())
		{
			http_response response{ m_http_service->get(std::format("https://api.github.com/repos/{}/{}/releases", m_owner, m_repo)) };
			if (response.is_success())
			{
				nlohmann::json json{ response.get_content_as_json() };
				std::ofstream file{ m_cache_releases_path };
				file << json.dump(4);
				for (const nlohmann::json& release : json)
				{
					m_releases.emplace_back(release);
				}
				file.close();
			}
		}
		return m_releases;
	}
}