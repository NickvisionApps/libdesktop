#pragma once

#include <any>
#include <concepts>
#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <utility>
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
		template <typename T, typename TFirst, typename... TRest>
		    requires std::is_class_v<T> && std::constructible_from<T, TFirst, TRest...>
		void add(service_scope scope, TFirst&& first, TRest&&... rest)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(T)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(T).name()));
			}
			m_services.emplace(typeid(T), service_entry{ scope, [first = std::forward<TFirst>(first), ... rest = std::forward<TRest>(rest)]() mutable
			{
				return std::make_any<std::shared_ptr<T>>(std::make_shared<T>(first, rest...));
			} });
		}
		template <typename T>
		    requires std::is_class_v<T>
		void add(service_scope scope, std::function<std::shared_ptr<T>()> factory)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(T)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(T).name()));
			}
			m_services.emplace(typeid(T), service_entry{ scope, [f = std::move(factory)]() mutable
			{
				return std::make_any(f());
			} });
		}
		template <typename T>
		    requires std::is_class_v<T>
		void add(service_scope scope)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(T)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(T).name()));
			}
			m_services.emplace(typeid(T), service_entry{ scope, make_resolving_factory<T>() });
		}
		template <typename T>
		    requires std::is_class_v<T>
		void add(std::shared_ptr<T> instance)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(T)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(T).name()));
			}
			m_services.emplace(typeid(T), service_entry{ service_scope::singleton, nullptr, std::make_any<std::shared_ptr<T>>(std::move(instance)) });
		}
		template <typename TInterface, typename TImpl, typename TFirst, typename... TRest>
		    requires std::is_class_v<TInterface> && std::derived_from<TImpl, TInterface> && std::constructible_from<TImpl, TFirst, TRest...> &&
		             (!std::same_as<TInterface, TImpl>)
		void add(service_scope scope, TFirst&& first, TRest&&... rest)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(TInterface)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(TInterface).name()));
			}
			m_services.emplace(typeid(TInterface), service_entry{ scope, [first = std::forward<TFirst>(first), ... rest = std::forward<TRest>(rest)]() mutable
			{
				return std::make_any<std::shared_ptr<TInterface>>(std::make_shared<TImpl>(first, rest...));
			} });
		}
		template <typename TInterface, typename TImpl>
		    requires std::is_class_v<TInterface> && std::derived_from<TImpl, TInterface> && (!std::same_as<TInterface, TImpl>)
		void add(service_scope scope, std::function<std::shared_ptr<TImpl>()> factory)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(TInterface)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(TInterface).name()));
			}
			m_services.emplace(typeid(TInterface), service_entry{ scope, [f = std::move(factory)]() mutable
			{
				return std::make_any<std::shared_ptr<TInterface>>(f());
			} });
		}
		template <typename TInterface, typename TImpl>
		    requires std::is_class_v<TInterface> && std::derived_from<TImpl, TInterface> && (!std::same_as<TInterface, TImpl>)
		void add(service_scope scope)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(TInterface)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(TInterface).name()));
			}
			m_services.emplace(typeid(TInterface), service_entry{ scope, make_resolving_factory<TInterface, TImpl>() });
		}
		template <typename TInterface, typename TImpl>
		    requires std::is_class_v<TInterface> && std::derived_from<TImpl, TInterface> && (!std::same_as<TInterface, TImpl>)
		void add(std::shared_ptr<TImpl> instance)
		{
			std::scoped_lock lock{ m_mutex };
			if (m_services.contains(typeid(TInterface)))
			{
				throw std::runtime_error(std::format("Service already registered ({})", typeid(TInterface).name()));
			}
			m_services.emplace(typeid(TInterface),
			                   service_entry{ service_scope::singleton, nullptr, std::make_any<std::shared_ptr<TInterface>>(std::move(instance)) });
		}
		template <typename T>
		    requires std::is_class_v<T>
		bool contains() const
		{
			std::scoped_lock lock{ m_mutex };
			return m_services.contains(typeid(T));
		}
		template <typename T>
		    requires std::is_class_v<T>
		void remove()
		{
			std::scoped_lock lock{ m_mutex };
			m_services.erase(typeid(T));
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
			    : m_scope{ scope },
			      m_factory{ std::move(factory) },
			      m_instance{ std::move(instance) }
			{
			}
			service_entry(const service_entry&) = default;
			service_entry(service_entry&&) noexcept = default;
			service_entry& operator=(const service_entry&) = default;
			service_entry& operator=(service_entry&&) noexcept = default;

		private:
			friend class service_collection;
			service_scope m_scope;
			mutable std::function<std::any()> m_factory;
			mutable std::optional<std::any> m_instance;
		};

		std::any get_impl(std::type_index type) const override;
		template <typename T>
		    requires std::is_class_v<T>
		std::function<std::any()> make_resolving_factory()
		{
			if constexpr (has_dependencies<T>)
			{
				return [this]<typename... TDeps>(std::tuple<TDeps...>*)
				{
					return std::function<std::any()>{ [this]()
					{
						return std::make_any<std::shared_ptr<T>>(std::make_shared<T>(this->get_required<TDeps>()...));
					} };
				}(static_cast<T::dependencies*>(nullptr));
			}
			else
			{
				return []()
				{
					return std::make_any<std::shared_ptr<T>>(std::make_shared<T>());
				};
			}
		}
		template <typename TInterface, typename TImpl>
		    requires std::is_class_v<TInterface> && std::derived_from<TImpl, TInterface>
		std::function<std::any()> make_resolving_factory()
		{
			if constexpr (has_dependencies<TImpl>)
			{
				return [this]<typename... TDeps>(std::tuple<TDeps...>*)
				{
					return std::function<std::any()>{ [this]()
					{
						return std::make_any<std::shared_ptr<TInterface>>(std::make_shared<TImpl>(this->get_required<TDeps>()...));
					} };
				}(static_cast<TImpl::dependencies*>(nullptr));
			}
			else
			{
				return []()
				{
					return std::make_any<std::shared_ptr<TInterface>>(std::make_shared<TImpl>());
				};
			}
		}
		mutable std::mutex m_mutex;
		std::unordered_map<std::type_index, service_entry> m_services;
	};
}
