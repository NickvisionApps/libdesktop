#pragma once

#include <cpr/cpr.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace desktop::network::web
{
	bool download_file(const std::string& url, const std::filesystem::path& destination, bool overwrite = true, const cpr::ProgressCallback& progress = {});
	nlohmann::json get_json(const std::string& url);
	template <typename T>
	std::optional<T> get_json(const std::string& url)
	{
		nlohmann::json json{ get_json(url) };
		if (!json.is_null() && !json.empty())
		{
			try
			{
				return json.get<T>();
			}
			catch (...)
			{
				return std::nullopt;
			}
		}
		return std::nullopt;
	}
	bool is_existing_url(const std::string& url);
	bool is_valid_url(const std::string& url);
}