#pragma once

#include <concepts>
#include <exception>
#include <filesystem>
#include <memory>
#include "app/log_type.h"
#include "host_options.h"
#include "lifetime_service.h"
#include "services/service_collection.h"

namespace desktop::hosting
{
	class host
	{
	public:
		host(host_options options);
		~host() = default;
		template <typename... TArgs>
		    requires std::constructible_from<host_options, TArgs...> &&
		             (sizeof...(TArgs) > 1 || (sizeof...(TArgs) == 1 && (!std::same_as<std::remove_cvref_t<TArgs>, host_options> && ...) &&
		                                       (!std::same_as<std::remove_cvref_t<TArgs>, host> && ...)))
		host(TArgs&&... args)
		    : host{ host_options{ std::forward<TArgs>(args)... } }
		{
		}
		host(const host&) = default;
		host(host&&) noexcept = default;
		std::shared_ptr<services::service_collection>& get_services();
		std::exception_ptr run();
		void use_github_updates();
		template <typename T>
		    requires std::is_base_of_v<lifetime_service, T>
		void use_lifetime()
		{
			m_services->add<lifetime_service, T>(services::service_scope::singleton);
		}
#ifdef NDEBUG
		void use_logging(app::log_type minimum = app::log_type::info, const std::filesystem::path& path = {});
#else
		void use_logging(app::log_type minimum = app::log_type::debug, const std::filesystem::path& path = {});
#endif
		host& operator=(const host&) = default;
		host& operator=(host&&) noexcept = default;

	private:
		host_options m_options;
		std::shared_ptr<services::service_collection> m_services;
	};
}