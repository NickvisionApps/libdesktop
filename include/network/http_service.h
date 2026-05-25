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

namespace desktop::network
{
	class http_service
	{
	public:
		http_service();
		~http_service();
		http_service(const http_service&) = delete;
		http_service(http_service&&) = delete;
		http_response del(const std::string& url) const;
		void disable_ssl_verification();
		bool download_file(const std::string& url, const std::filesystem::path& destination, bool overwrite = true,
		                   const std::function<void(const download_progress&)>& progress = {}) const;
		void enable_ssl_verification();
		http_response get(const std::string& url) const;
		bool is_ssl_verification_enabled() const;
		bool is_valid_url(const std::string& url) const;
		http_response patch(const std::string& url, const nlohmann::json& json) const;
		http_response patch(const std::string& url, const std::vector<std::byte>& data) const;
		http_response patch(const std::string& url, const std::filesystem::path& file_path) const;
		http_response patch(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form) const;
		bool ping(const std::string& url) const;
		http_response post(const std::string& url, const nlohmann::json& json) const;
		http_response post(const std::string& url, const std::vector<std::byte>& data) const;
		http_response post(const std::string& url, const std::filesystem::path& file_path) const;
		http_response post(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form) const;
		http_response put(const std::string& url, const nlohmann::json& json) const;
		http_response put(const std::string& url, const std::vector<std::byte>& data) const;
		http_response put(const std::string& url, const std::filesystem::path& file_path) const;
		http_response put(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form) const;
		http_service& operator=(const http_service&) = delete;
		http_service& operator=(http_service&&) = delete;

	private:
		bool m_ssl_verification{ true };
	};
}
