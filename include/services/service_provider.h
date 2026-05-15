#pragma once

#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>

namespace desktop::services
{
	template <typename T>
	concept has_dependencies = requires { typename T::dependencies; };

	class service_provider
	{
	public:
		service_provider() = default;
		virtual ~service_provider() = default;
		service_provider(const service_provider&) = default;
		service_provider(service_provider&&) = default;
		template <typename T>
		    requires std::is_class_v<T>
		std::shared_ptr<T> get() const
		{
			std::any result{ get_impl(typeid(T)) };
			if (!result.has_value())
			{
				return nullptr;
			}
			return std::any_cast<std::shared_ptr<T>>(result);
		}
		template <typename T>
		    requires std::is_class_v<T>
		std::shared_ptr<T> get_required() const
		{
			auto svc{ get<T>() };
			if (!svc)
			{
				throw std::runtime_error("Required service not registered: " + std::string(typeid(T).name()));
			}
			return svc;
		}
		service_provider& operator=(const service_provider&) = default;
		service_provider& operator=(service_provider&&) = default;

	protected:
		virtual std::any get_impl(std::type_index type) const = 0;
	};
}