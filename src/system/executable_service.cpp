#include "system/executable_service.h"
#include <format>
#include "filesystem/user_directories.h"
#include "helpers/file.h"
#include "helpers/string_manip.h"
#include "system/environment.h"
#include "system/process.h"

using namespace desktop::app;
using namespace desktop::filesystem;
using namespace desktop::helpers;
using namespace desktop::network;
using namespace desktop::updates;

namespace desktop::system
{
	executable_service::executable_service(std::string executable_name, version bundled_version, std::string asset_name,
	                                       std::shared_ptr<configuration_service> configuration_service, std::shared_ptr<update_service> stable_update_service,
	                                       std::shared_ptr<update_service> preview_update_service)
	    : m_executable_name{ std::move(executable_name) },
	      m_bundled_version{ std::move(bundled_version) },
	      m_asset_name{ std::move(asset_name) },
	      m_configuration_service{ std::move(configuration_service) },
	      m_update_service{ std::move(stable_update_service) },
	      m_preview_update_service{ std::move(preview_update_service) }
	{
	}

	bool executable_service::download_update(const version& version, const std::function<void(const download_progress&)>& progress)
	{
		std::scoped_lock lock{ m_mutex };
		bool is_zip{ std::filesystem::path(string_manip::lower(m_asset_name)).extension() == ".zip" };
#ifdef _WIN32
		std::filesystem::path download_path{ user_directories::get_local_data() / std::format("{}.{}", m_executable_name, is_zip ? "zip" : "exe") };
#else
		std::filesystem::path download_path{ user_directories::get_local_data() / std::format("{}.{}", m_executable_name, is_zip ? "zip" : "bin") };
#endif
		bool res{ false };
		if (version.is_preview() && m_preview_update_service)
		{
			res = m_preview_update_service->download_asset(version, m_asset_name, download_path, true, progress);
		}
		else
		{
			res = m_update_service->download_asset(version, m_asset_name, download_path, true, progress);
		}
		if (!res)
		{
			return false;
		}
		std::string config_key{ std::format("installed_{}_appversion", m_executable_name) };
#ifdef _WIN32
		std::filesystem::path executable_path{ user_directories::get_local_data() / std::format("{}.exe", m_executable_name) };
#else
		std::filesystem::path executable_path{ user_directories::get_local_data() / std::format("{}.bin", m_executable_name) };
#endif
		if (is_zip)
		{
			if (file::extract_archive(download_path, user_directories::get_local_data()))
			{
				std::filesystem::remove(download_path);
			}
		}
		if (!std::filesystem::exists(executable_path))
		{
			return false;
		}
		m_configuration_service->set(config_key, version);
#ifndef _WIN32
		process proc{ "chmod", { "0755", executable_path.string() } };
		proc.start();
		proc.wait_for_exit();
		res = proc.get_exit_code() == 0;
#endif
		return res;
	}

	process_result executable_service::execute(const std::vector<std::string>& arguments) const
	{
		process proc{ get_executable_path(), arguments };
		if (proc.start())
		{
			proc.wait_for_exit();
			return proc.get_result();
		}
		return {};
	}

	const version& executable_service::get_bundled_version() const
	{
		return m_bundled_version;
	}

	std::filesystem::path executable_service::get_executable_path() const
	{
		std::scoped_lock lock{ m_mutex };
		if (std::filesystem::exists(m_executable_path))
		{
			return m_executable_path;
		}
		std::string config_key{ std::format("installed_{}_appversion", m_executable_name) };
		if (m_configuration_service->get(config_key, m_bundled_version) > m_bundled_version)
		{
			std::filesystem::path local{ environment::find_dependency(m_executable_name, dependency_search_option::local) };
			if (std::filesystem::exists(local))
			{
				m_executable_path = local;
				return m_executable_path;
			}
			m_configuration_service->set(config_key, m_bundled_version);
		}
		m_executable_path = environment::find_dependency(m_executable_name, dependency_search_option::global);
		return std::filesystem::exists(m_executable_path) ? m_executable_path : m_executable_name;
	}

	version executable_service::get_executable_version(const std::string& version_argument) const
	{
		process proc{ get_executable_path(), { version_argument } };
		if (proc.start())
		{
			proc.wait_for_exit();
			if (proc.get_exit_code() == 0)
			{
				std::optional<version> ver{ version::parse(string_manip::trim(proc.get_standard_output())) };
				if (ver.has_value())
				{
					return *ver;
				}
			}
		}
		return {};
	}

	const version& executable_service::get_latest_version(bool preview) const
	{
		std::scoped_lock lock{ m_mutex };
		if (preview)
		{
			if (m_latest_preview_version.empty())
			{
				std::optional<version> latest{ m_preview_update_service ? m_preview_update_service->get_latest_version(false)
					                                                    : m_update_service->get_latest_version(true) };
				if (latest.has_value())
				{
					m_latest_preview_version = *latest;
				}
			}
			return m_latest_preview_version;
		}
		if (m_latest_stable_version.empty())
		{
			std::optional<version> latest{ m_update_service->get_latest_version(false) };
			if (latest.has_value())
			{
				m_latest_stable_version = *latest;
			}
		}
		return m_latest_stable_version;
	}

	version executable_service::get_installed_version() const
	{
		return m_configuration_service->get(std::format("installed_{}_appversion", m_executable_name), m_bundled_version);
	}
}