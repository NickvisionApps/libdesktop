#pragma once

#include <cstdint>
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
		int64_t get_size() const;
		void set_size(int64_t size);
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
		int64_t m_size;
		std::string m_digest;
		std::string m_browser_download_url;
	};

	inline void to_json(nlohmann::json& j, const release_asset& a)
	{
		j = {
			{ "url", a.get_url() },
			{ "name", a.get_name() },
			{ "size", a.get_size() },
			{ "digest", a.get_digest() },
			{ "browser_download_url", a.get_browser_download_url() }
		};
	}

	inline void from_json(const nlohmann::json& j, release_asset& a)
	{
		a = release_asset{ j };
	}
}