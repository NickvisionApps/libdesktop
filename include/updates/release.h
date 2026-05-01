#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "release_asset.h"

namespace desktop::updates
{
	class release
	{
	public:
		release() = default;
		~release() = default;
		release(const nlohmann::json& json);
		release(const release&) = default;
		release(release&&) = default;
		const std::string& get_tag_name() const;
		void set_tag_name(const std::string& tag_name);
		bool is_prerelease() const;
		void set_prerelease(bool prerelease);
		bool is_draft() const;
		void set_draft(bool draft);
		const std::vector<release_asset>& get_assets() const;
		void set_assets(const std::vector<release_asset>& assets);
		bool empty() const;
		release& operator=(const release&) = default;
		release& operator=(release&&) = default;
		operator bool() const;

	private:
		std::string m_url;
		std::string m_tag_name;
		bool m_prerelease;
		bool m_draft;
		std::vector<release_asset> m_assets;
	};
}