#include "services/service_collection.h"
#include <stdexcept>
#include <unordered_set>

namespace desktop::services
{
	std::any service_collection::get_service_impl(std::type_index type) const
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
		service_scope scope{ service_scope::singleton };
		std::function<std::any()> factory;
		{
			std::scoped_lock lock{ m_mutex };
			auto it{ m_services.find(type) };
			if (it == m_services.end())
			{
				return {};
			}
			const service_entry& entry{ it->second };
			if (entry.scope == service_scope::singleton && entry.instance.has_value())
			{
				return entry.instance.value();
			}
			scope = entry.scope;
			factory = entry.factory;
		}
		if (!factory)
		{
			return {};
		}
		in_progress.insert(type);
		std::any result;
		try
		{
			result = factory();
			in_progress.erase(type);
		}
		catch (...)
		{
			in_progress.erase(type);
			throw;
		}
		if (scope == service_scope::singleton)
		{
			std::scoped_lock lock{ m_mutex };
			auto it{ m_services.find(type) };
			if (it != m_services.end())
			{
				if (!it->second.instance.has_value())
				{
					it->second.instance = result;
				}
				else
				{
					result = it->second.instance.value();
				}
			}
		}
		return result;
	}
}