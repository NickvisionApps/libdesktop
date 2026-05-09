#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>
#include "download_progress.h"
#include "http_response.h"
#include "services/service.h"

namespace desktop::network
{
	class http_service : public services::service
	{
	public:
		http_service();
		~http_service() override;
		http_service(const http_service&) = delete;
		http_service(http_service&&) = delete;
		http_response del(const std::string& url);
		bool download_file(const std::string& url, const std::filesystem::path& destination, bool overwrite = true,
		                   const std::function<void(download_progress)>& progress = {});
		http_response get(const std::string& url);
		bool is_valid_url(const std::string& url);
		http_response patch(const std::string& url, const nlohmann::json& json);
		http_response patch(const std::string& url, const std::vector<std::byte>& data);
		http_response patch(const std::string& url, const std::filesystem::path& file_path);
		http_response patch(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form);
		http_response post(const std::string& url, const nlohmann::json& json);
		http_response post(const std::string& url, const std::vector<std::byte>& data);
		http_response post(const std::string& url, const std::filesystem::path& file_path);
		http_response post(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form);
		http_response put(const std::string& url, const nlohmann::json& json);
		http_response put(const std::string& url, const std::vector<std::byte>& data);
		http_response put(const std::string& url, const std::filesystem::path& file_path);
		http_response put(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form);
		http_service& operator=(const http_service&) = delete;
		http_service& operator=(http_service&&) = delete;
	};
}
