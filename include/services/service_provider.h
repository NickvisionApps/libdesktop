#pragma once

#include <any>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include "service.h"

namespace desktop::services
{
	class service_provider : public service
	{
	public:
		service_provider() = default;
		~service_provider() override = default;
		service_provider(const service_provider&) = delete;
		service_provider(service_provider&&) = delete;

		template <typename TService>
		    requires is_service<TService>
		std::shared_ptr<TService> get_service() const
		{
			std::any result{ get_service_impl(typeid(TService)) };
			if (!result.has_value())
			{
				return nullptr;
			}
			return std::any_cast<std::shared_ptr<TService>>(result);
		}

		template <typename TService>
		    requires is_service<TService>
		std::shared_ptr<TService> get_required_service() const
		{
			auto svc{ get_service<TService>() };
			if (!svc)
			{
				throw std::runtime_error("Required service not registered: " + std::string(typeid(TService).name()));
			}
			return svc;
		}

		service_provider& operator=(const service_provider&) = delete;
		service_provider& operator=(service_provider&&) = delete;

	protected:
		virtual std::any get_service_impl(std::type_index type) const = 0;
	};
}