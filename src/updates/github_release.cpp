#include "updates/github_release.h"

namespace desktop::updates
{
	github_release::github_release(const nlohmann::json& json)
	    : m_url{ json.value("url", "") },
	      m_tag_name{ json.value("tag_name", "") },
	      m_prerelease{ json.value("prerelease", false) },
	      m_draft{ json.value("draft", false) }
	{
		for (const nlohmann::json& asset_json : json["assets"])
		{
			if (!asset_json.is_object())
			{
				continue;
			}
			github_release_asset asset{ asset_json };
			if (!asset.empty())
			{
				m_assets.emplace_back(asset);
			}
		}
	}

	const std::string& github_release::get_tag_name() const
	{
		return m_tag_name;
	}

	void github_release::set_tag_name(const std::string& tag_name)
	{
		m_tag_name = tag_name;
	}

	bool github_release::is_prerelease() const
	{
		return m_prerelease;
	}

	void github_release::set_prerelease(bool prerelease)
	{
		m_prerelease = prerelease;
	}

	bool github_release::is_draft() const
	{
		return m_draft;
	}

	void github_release::set_draft(bool draft)
	{
		m_draft = draft;
	}

	const std::vector<github_release_asset>& github_release::get_assets() const
	{
		return m_assets;
	}

	void github_release::set_assets(const std::vector<github_release_asset>& assets)
	{
		m_assets = assets;
	}

	bool github_release::empty() const
	{
		return m_url.empty() && m_tag_name.empty() && m_assets.empty();
	}

	github_release::operator bool() const
	{
		return !empty();
	}
}