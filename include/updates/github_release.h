#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "github_release_asset.h"

namespace desktop::updates
{
	class github_release
	{
	public:
		github_release() = default;
		~github_release() = default;
		github_release(const nlohmann::json& json);
		github_release(const github_release&) = default;
		github_release(github_release&&) = default;
		const std::string& get_tag_name() const;
		void set_tag_name(const std::string& tag_name);
		bool is_prerelease() const;
		void set_prerelease(bool prerelease);
		bool is_draft() const;
		void set_draft(bool draft);
		const std::vector<github_release_asset>& get_assets() const;
		void set_assets(const std::vector<github_release_asset>& assets);
		bool empty() const;
		github_release& operator=(const github_release&) = default;
		github_release& operator=(github_release&&) = default;
		operator bool() const;

		friend void to_json(nlohmann::json& j, const github_release& r)
		{
			j = { { "tag_name", r.get_tag_name() }, { "prerelease", r.is_prerelease() }, { "draft", r.is_draft() }, { "assets", r.get_assets() } };
		}

		friend void from_json(const nlohmann::json& j, github_release& r)
		{
			r = github_release{ j };
		}

	private:
		std::string m_url;
		std::string m_tag_name;
		bool m_prerelease{ false };
		bool m_draft{ false };
		std::vector<github_release_asset> m_assets;
	};
}