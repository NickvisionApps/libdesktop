#pragma once

#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "app/app_info.h"
#include "services/service.h"

namespace desktop::app
{
	class translation_service : public services::service
	{
	public:
		using dependencies = std::tuple<app_info>;
		translation_service(const std::shared_ptr<app_info>& info);
		~translation_service() override = default;
		translation_service(const translation_service&) = delete;
		translation_service(translation_service&&) = delete;
		std::string_view get_language() const;
		bool set_language(std::string_view language);
		const std::vector<std::string>& get_available_languages() const;
		const char* _(const char* msgid) const noexcept;
		template <typename... Args>
		std::string _(const char* msgid, Args&&... args) const noexcept
		{
			return std::vformat(_(msgid), std::make_format_args(args...));
		}
		const char* _n(const char* msgid, const char* msgid_plural, unsigned long n) const noexcept;
		template <typename... Args>
		std::string _n(const char* msgid, const char* msgid_plural, unsigned long n, Args&&... args) const noexcept
		{
			return std::vformat(_n(msgid, msgid_plural, n), std::make_format_args(args...));
		}
		const char* _p(const char* context, const char* msgid) const noexcept;
		template <typename... Args>
		std::string _p(const char* context, const char* msgid, Args&&... args) const noexcept
		{
			return std::vformat(_p(context, msgid), std::make_format_args(args...));
		}
		const char* _pn(const char* context, const char* msgid, const char* msgid_plural, unsigned long n) const noexcept;
		template <typename... Args>
		std::string _pn(const char* context, const char* msgid, const char* msgid_plural, unsigned long n, Args&&... args) const noexcept
		{
			return std::vformat(_pn(context, msgid, msgid_plural, n), std::make_format_args(args...));
		}
		translation_service& operator=(const translation_service&) = delete;
		translation_service& operator=(translation_service&&) = delete;

	private:
		std::string m_domain_name;
		std::string m_language;
		bool m_translations_off;
		mutable std::mutex m_languages_mutex;
		mutable std::vector<std::string> m_available_languages;
	};
}