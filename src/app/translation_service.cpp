#include "app/translation_service.h"
#include <algorithm>
#include <clocale>
#include <filesystem>
#include <libintl.h>
#include "helpers/string_manip.h"
#include "system/environment.h"

using namespace desktop::helpers;
using namespace desktop::system;

namespace desktop::app
{
	translation_service::translation_service(const std::shared_ptr<app_info>& info)
	    : m_domain_name{ string_manip::lower(string_manip::replace_all(info->get_english_short_name(), " ", "")) },
	      m_language{ "C" },
	      m_translations_off{ true }
	{
		setlocale(LC_ALL, "");
#ifdef _WIN32
		wbindtextdomain(m_domain_name.c_str(), environment::get_executable_directory().c_str());
#else
		bindtextdomain(m_domain_name.c_str(), environment::get_executable_directory().string().c_str());
#endif
		bind_textdomain_codeset(m_domain_name.c_str(), "UTF-8");
		textdomain(m_domain_name.c_str());
	}

	std::string_view translation_service::get_language() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_language;
	}

	bool translation_service::set_language(std::string_view language)
	{
		if (language.empty())
		{
			environment::clear_variable("LANGUAGE");
			std::scoped_lock lock{ m_mutex };
			m_translations_off = false;
			m_language = language;
			return true;
		}
		if (language == "C")
		{
			std::scoped_lock lock{ m_mutex };
			m_translations_off = true;
			m_language = language;
			return true;
		}
		const std::vector<std::string>& langs{ get_available_languages() };
		if (std::ranges::find(langs, language) == langs.end())
		{
			return false;
		}
		environment::set_variable("LANGUAGE", std::string{ language });
		std::scoped_lock lock{ m_mutex };
		m_translations_off = false;
		m_language = language;
		return true;
	}

	const std::vector<std::string>& translation_service::get_available_languages() const
	{
		std::scoped_lock lock{ m_mutex };
		if (m_available_languages.empty())
		{
			for (const std::filesystem::directory_entry& e : std::filesystem::directory_iterator(environment::get_executable_directory()))
			{
				if (e.is_directory() && std::filesystem::exists(e.path() / "LC_MESSAGES" / (m_domain_name + ".mo")))
				{
					m_available_languages.push_back(e.path().filename().string());
				}
			}
			std::ranges::sort(m_available_languages);
		}
		return m_available_languages;
	}

	const char* translation_service::_(const char* msgid) const noexcept
	{
		if (m_translations_off)
		{
			return msgid;
		}
		return ::dgettext(m_domain_name.c_str(), msgid);
	}

	const char* translation_service::_n(const char* msgid, const char* msgid_plural, unsigned long n) const noexcept
	{
		if (m_translations_off)
		{
			return n == 1 ? msgid : msgid_plural;
		}
		return ::dngettext(m_domain_name.c_str(), msgid, msgid_plural, n);
	}

	const char* translation_service::_p(const char* context, const char* msgid) const noexcept
	{
		if (m_translations_off)
		{
			return msgid;
		}
		try
		{
			std::string ctx_key{ std::string{ context } + "\004" + msgid };
			const char* translation{ ::dcgettext(m_domain_name.c_str(), ctx_key.c_str(), LC_MESSAGES) };
			return translation == ctx_key.c_str() ? msgid : translation;
		}
		catch (...)
		{
			return msgid;
		}
	}

	const char* translation_service::_pn(const char* context, const char* msgid, const char* msgid_plural, unsigned long n) const noexcept
	{
		if (m_translations_off)
		{
			return n == 1 ? msgid : msgid_plural;
		}
		try
		{
			std::string ctx_key{ std::string{ context } + "\004" + msgid };
			const char* translation{ ::dcngettext(m_domain_name.c_str(), ctx_key.c_str(), msgid_plural, n, LC_MESSAGES) };
			if (translation == ctx_key.c_str() || translation == msgid_plural)
			{
				return n == 1 ? msgid : msgid_plural;
			}
			return translation;
		}
		catch (...)
		{
			return n == 1 ? msgid : msgid_plural;
		}
	}
}