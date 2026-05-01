#include "app/app_info.h"
#include <ranges>
#include <sstream>
#include <string_view>
#include <maddy/parser.h>

namespace desktop::app
{
	app_info::app_info(const std::string& id, const std::string& name, const std::string english_short_name)
		: m_id{ id },
		m_name{ name },
		m_english_short_name{ english_short_name }
	{

	}

	const std::unordered_map<std::string, std::string>& app_info::get_artists() const
	{
		return m_artists;
	}

	void app_info::add_artist(const std::string& name, const std::string& url)
	{
		m_artists[name] = url;
	}

	const std::string& app_info::get_changelog() const
	{
		return m_changelog;
	}

	const std::string& app_info::get_changelog_html() const
	{
		return m_changelog_html;
	}

	void app_info::set_changelog(const std::string& changelog)
	{
		m_changelog = changelog;
		if (m_changelog.empty())
		{
			m_changelog_html = "";
		}
		else
		{

		}
	}

	const std::string& app_info::get_description() const
	{
		return m_description;
	}

	void app_info::set_description(const std::string& description)
	{
		m_description = description;
	}

	const std::unordered_map<std::string, std::string>& app_info::get_designers() const
	{
		return m_designers;
	}

	void app_info::add_designer(const std::string& name, const std::string& url)
	{
		m_designers[name] = url;
	}

	const std::unordered_map<std::string, std::string>& app_info::get_developers() const
	{
		return m_developers;
	}

	void app_info::add_developer(const std::string& name, const std::string& url)
	{
		m_developers[name] = url;
	}

	const std::string& app_info::get_discussions_url() const
	{
		return m_discussions_url;
	}

	void app_info::set_discussions_url(const std::string& discussions_url)
	{
		m_discussions_url = discussions_url;
	}

	const std::string& app_info::get_english_short_name() const
	{
		return m_english_short_name;
	}

	const std::unordered_map<std::string, std::string>& app_info::get_extra_links() const
	{
		return m_extra_links;
	}

	void app_info::add_extra_link(const std::string& name, const std::string& url)
	{
		m_extra_links[name] = url;
	}

	const std::string& app_info::get_id() const
	{
		return m_id;
	}

	const std::string& app_info::get_issues_url() const
	{
		return m_issues_url;
	}

	void app_info::set_issues_url(const std::string& issues_url)
	{
		m_issues_url = issues_url;
	}

	const std::string& app_info::get_name() const
	{
		return m_name;
	}

	bool app_info::is_portable() const
	{
		return m_portable;
	}

	void app_info::set_portable(bool portable)
	{
		m_portable = portable;
	}

	const std::string& app_info::get_short_name() const
	{
		return m_short_name;
	}

	void app_info::set_short_name(const std::string& short_name)
	{
		m_short_name = short_name;
	}

	const std::string& app_info::get_source_url() const
	{
		return m_source_url;
	}

	void app_info::set_source_url(const std::string& source_url)
	{
		m_source_url = source_url;
	}

	const std::string& app_info::get_translation_credits() const
	{
		return m_translation_credits;
	}

	void app_info::set_translation_credits(const std::string& translation_credits)
	{
		m_translation_credits = translation_credits;
	}

	const updates::version& app_info::get_version() const
	{
		return m_version;
	}

	void app_info::set_version(const updates::version& version)
	{
		m_version = version;
	}
}