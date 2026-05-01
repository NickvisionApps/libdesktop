#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace desktop::updates
{
	class release_asset
	{
	public:
		release_asset() = default;
		~release_asset() = default;
		release_asset(const nlohmann::json& json);
		release_asset(const release_asset&) = default;
		release_asset(release_asset&&) = default;
		const std::string& get_url() const;
		void set_url(const std::string& url);
		const std::string& get_name() const;
		void set_name(const std::string& name);
		long get_size() const;
		void set_size(long size);
		const std::string& get_digest() const;
		void set_digest(const std::string& digest);
		const std::string& get_browser_download_url() const;
		void set_browser_download_url(const std::string& browser_download_url);
		bool empty() const;
		release_asset& operator=(const release_asset&) = default;
		release_asset& operator=(release_asset&&) = default;
		operator bool() const;

	private:
		std::string m_url;
		std::string m_name;
		long m_size;
		std::string m_digest;
		std::string m_browser_download_url;
	};
}