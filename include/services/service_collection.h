#pragma once

#include <any>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include "service.h"
#include "service_provider.h"
#include "service_scope.h"

namespace desktop::services
{
	class service_collection : public service_provider, public std::enable_shared_from_this<service_collection>
	{
	public:
		service_collection() = default;
		~service_collection() override = default;
		service_collection(const service_collection&) = delete;
		service_collection(service_collection&&) = delete;

		template<typename TService, typename TFirst, typename... TRest>
			requires is_service<TService> && std::constructible_from<TService, TFirst, TRest...>
		void add_service(service_scope scope, TFirst&& first, TRest&&... rest)
		{
			std::lock_guard lock{ m_mutex };
			if(m_services.contains(typeid(TService)))
			{
				return;
			}
			m_services.emplace(typeid(TService), service_entry{ scope, [first = std::forward<TFirst>(first), ... rest = std::forward<TRest>(rest)]() mutable
			{
				return std::make_any<std::shared_ptr<TService>>(std::make_shared<TService>(first, rest...));
			} });
		}

		template<typename TInterface, typename TImpl, typename TFirst, typename... TRest>
			requires is_service<TInterface> && std::derived_from<TImpl, TInterface> && std::constructible_from<TImpl, TFirst, TRest...>
		void add_service(service_scope scope, TFirst&& first, TRest&&... rest)
		{
			std::lock_guard lock{ m_mutex };
			if(m_services.contains(typeid(TInterface)))
			{
				return;
			}
			m_services.emplace(typeid(TInterface), service_entry{ scope, [first = std::forward<TFirst>(first), ... rest = std::forward<TRest>(rest)]() mutable
			{
				return std::make_any<std::shared_ptr<TInterface>>(std::make_shared<TImpl>(first, rest...));
			} });
		}

		template<typename TService>
			requires is_service<TService>
		void add_service(service_scope scope, std::function<std::shared_ptr<TService>()> factory)
		{
			std::lock_guard lock{ m_mutex };
			if(m_services.contains(typeid(TService)))
			{
				return;
			}
			m_services.emplace(typeid(TService), service_entry{ scope, [f = std::move(factory)]() mutable
			{
				return std::make_any<std::shared_ptr<TService>>(f());
			} });
		}

		template<typename TInterface, typename TImpl>
			requires is_service<TInterface> && std::derived_from<TImpl, TInterface>
		void add_service(service_scope scope, std::function<std::shared_ptr<TImpl>()> factory)
		{
			std::lock_guard lock{ m_mutex };
			if(m_services.contains(typeid(TInterface)))
			{
				return;
			}
			m_services.emplace(typeid(TInterface), service_entry{ scope, [f = std::move(factory)]() mutable
			{
				return std::make_any<std::shared_ptr<TInterface>>(f());
			} });
		}

		template<typename TInterface, typename TImpl>
			requires is_service<TInterface> && std::derived_from<TImpl, TInterface>
		void add_service(service_scope scope)
		{
			std::lock_guard lock{ m_mutex };
			if(m_services.contains(typeid(TInterface)))
			{
				return;
			}
			m_services.emplace(typeid(TInterface), service_entry{ scope, make_resolving_factory<TInterface, TImpl>(), std::nullopt });
		}

		template<typename TService>
			requires is_service<TService>
		void add_service(service_scope scope)
		{
			add_service<TService, TService>(scope);
		}

		template<typename TService>
			requires is_service<TService>
		bool contains() const
		{
			std::lock_guard lock{ m_mutex };
			return m_services.contains(typeid(TService));
		}

		template<typename TService>
			requires is_service<TService>
		void remove_service()
		{
			std::lock_guard lock{ m_mutex };
			m_services.erase(typeid(TService));
		}

		service_collection& operator=(const service_collection&) = delete;
		service_collection& operator=(service_collection&&) = delete;

	private:
		class service_entry
		{
		public:
			service_entry() = delete;
			~service_entry() = default;
			service_entry(service_scope scope, std::function<std::any()> factory, std::optional<std::any> instance = std::nullopt)
				: scope{ scope },
				factory{ std::move(factory) },
				instance{ std::move(instance) }
			{

			}

			service_entry(const service_entry&) = default;
			service_entry(service_entry&&) noexcept = default;

			std::any get_any_instance() const
			{
				if (scope == service_scope::singleton)
				{
					if (!instance.has_value())
					{
						if (!factory)
						{
							return {};
						}
						instance = factory();
					}
					return instance.value();
				}
				return factory();
			}

			service_entry& operator=(const service_entry&) = default;
			service_entry& operator=(service_entry&&) noexcept = default;

		private:
			service_scope scope;
			mutable std::function<std::any()> factory;
			mutable std::optional<std::any> instance;
		};

		std::any get_service_impl(std::type_index type) const override
		{
			if (type == typeid(service_provider))
			{
				return std::make_any<std::shared_ptr<service_provider>>(std::const_pointer_cast<service_collection>(shared_from_this()));
			}
			thread_local std::unordered_set<std::type_index> in_progress;
			if (in_progress.contains(type))
			{
				throw std::runtime_error("Circular dependency detected for service: " + std::string(type.name()));
			}
			std::lock_guard lock{ m_mutex };
			auto it{ m_services.find(type) };
			if (it == m_services.end())
			{
				return {};
			}
			in_progress.insert(type);
			try
			{
				std::any result{ it->second.get_any_instance() };
				in_progress.erase(type);
				return result;
			}
			catch (...)
			{
				in_progress.erase(type);
				throw;
			}
		}

		template<typename TInterface, typename TImpl>
			requires is_service<TInterface>&& std::derived_from<TImpl, TInterface>
		std::function<std::any()> make_resolving_factory()
		{
			if constexpr (has_dependencies<TImpl>)
			{
				return make_resolving_factory_deps<TInterface, TImpl>(static_cast<typename TImpl::dependencies*>(nullptr));
			}
			else
			{
				return []() 
				{
					return std::make_any<std::shared_ptr<TInterface>>(std::make_shared<TImpl>());
				};
			}
		}

		template<typename TInterface, typename TImpl, typename... TDeps>
			requires is_service<TInterface>&& std::derived_from<TImpl, TInterface>&& std::constructible_from<TImpl, std::shared_ptr<TDeps>...>
		std::function<std::any()> make_resolving_factory_deps(std::tuple<TDeps...>*)
		{
			return [this]() 
			{
				return std::make_any<std::shared_ptr<TInterface>>(std::make_shared<TImpl>(this->get_service<TDeps>()...));
			};
		}

		mutable std::recursive_mutex m_mutex;
		std::unordered_map<std::type_index, service_entry> m_services;
	};
}
