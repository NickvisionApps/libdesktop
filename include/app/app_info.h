#pragma once

#include <string>
#include <unordered_map>
#include "services/service.h"
#include "updates/version.h"

namespace desktop::app
{
	class app_info : public services::service
	{
	public:
		app_info(std::string id, std::string name, std::string english_short_name);
		~app_info() override = default;
		app_info(const app_info&) = delete;
		app_info(app_info&&) = delete;
		const std::unordered_map<std::string, std::string>& get_artists() const;
		void add_artist(const std::string& name, const std::string& url);
		const std::string& get_changelog() const;
		const std::string& get_changelog_html() const;
		void set_changelog(const std::string& changelog);
		const std::string& get_description() const;
		void set_description(const std::string& description);
		const std::unordered_map<std::string, std::string>& get_designers() const;
		void add_designer(const std::string& name, const std::string& url);
		const std::unordered_map<std::string, std::string>& get_developers() const;
		void add_developer(const std::string& name, const std::string& url);
		const std::string& get_discussions_url() const;
		void set_discussions_url(const std::string& discussions_url);
		const std::string& get_english_short_name() const;
		const std::unordered_map<std::string, std::string>& get_extra_links() const;
		void add_extra_link(const std::string& name, const std::string& url);
		const std::string& get_id() const;
		const std::string& get_issues_url() const;
		void set_issues_url(const std::string& issues_url);
		const std::string& get_name() const;
		bool is_portable() const;
		void set_portable(bool portable);
		const std::string& get_short_name() const;
		void set_short_name(const std::string& short_name);
		const std::string& get_source_url() const;
		void set_source_url(const std::string& source_url);
		const std::string& get_translation_credits() const;
		void set_translation_credits(const std::string& translation_credits);
		const updates::version& get_version() const;
		void set_version(const updates::version& version);
		app_info& operator=(const app_info&) = delete;
		app_info& operator=(app_info&&) = delete;

	private:
		std::unordered_map<std::string, std::string> m_artists;
		std::string m_changelog;
		std::string m_changelog_html;
		std::string m_description;
		std::unordered_map<std::string, std::string> m_designers;
		std::unordered_map<std::string, std::string> m_developers;
		std::string m_discussions_url;
		std::string m_english_short_name;
		std::unordered_map<std::string, std::string> m_extra_links;
		std::string m_id;
		std::string m_issues_url;
		std::string m_name;
		bool m_portable;
		std::string m_short_name;
		std::string m_source_url;
		std::string m_translation_credits;
		updates::version m_version;
	};
}