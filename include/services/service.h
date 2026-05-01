#pragma once

#include <concepts>

namespace desktop::services
{
	class service
	{
	public:
		service() = default;
		virtual ~service() = default;
		service(const service&) = delete;
		service(service&&) = delete;
		service& operator=(const service&) = delete;
		service& operator=(service&&) = delete;
	};

	template<typename T>
	concept has_dependencies = requires { typename T::dependencies; };

	template<typename T>
	concept is_service = std::derived_from<T, service>;
}