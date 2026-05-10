#include "updates/github_release_asset.h"

namespace desktop::updates
{
	github_release_asset::github_release_asset(const nlohmann::json& json)
	    : m_url{ json.value("url", "") },
	      m_name{ json.value("name", "") },
	      m_size{ json.value("size", int64_t{ 0 }) },
	      m_digest{ json.value("digest", "") },
	      m_browser_download_url{ json.value("browser_download_url", "") }
	{
	}

	const std::string& github_release_asset::get_url() const
	{
		return m_url;
	}

	void github_release_asset::set_url(const std::string& url)
	{
		m_url = url;
	}

	const std::string& github_release_asset::get_name() const
	{
		return m_name;
	}

	void github_release_asset::set_name(const std::string& name)
	{
		m_name = name;
	}

	int64_t github_release_asset::get_size() const
	{
		return m_size;
	}

	void github_release_asset::set_size(int64_t size)
	{
		m_size = size;
	}

	const std::string& github_release_asset::get_digest() const
	{
		return m_digest;
	}

	void github_release_asset::set_digest(const std::string& digest)
	{
		m_digest = digest;
	}

	const std::string& github_release_asset::get_browser_download_url() const
	{
		return m_browser_download_url;
	}

	void github_release_asset::set_browser_download_url(const std::string& browser_download_url)
	{
		m_browser_download_url = browser_download_url;
	}

	bool github_release_asset::empty() const
	{
		return m_url.empty() && m_name.empty() && m_size == 0L && m_digest.empty() && m_browser_download_url.empty();
	}

	github_release_asset::operator bool() const
	{
		return !empty();
	}
}