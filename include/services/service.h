#pragma once

#include <concepts>

namespace desktop::services
{
	class service
	{
	public:
		virtual ~service() = default;
	};

	template<typename T>
	concept has_dependencies = requires { typename T::dependencies; };

	template<typename T>
	concept is_service = std::derived_from<T, service>;
}