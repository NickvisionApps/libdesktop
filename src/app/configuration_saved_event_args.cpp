#include "app/configuration_saved_event_args.h"

namespace desktop::app
{
	configuration_saved_event_args::configuration_saved_event_args(const std::string& changed_property_name, const database_value& changed_property_new_value)
		: m_changed_property_name{ changed_property_name },
		m_changed_property_new_value{ changed_property_new_value }
	{

	}

	const std::string& configuration_saved_event_args::get_changed_property_name() const
	{
		return m_changed_property_name;
	}

	const database_value& configuration_saved_event_args::get_changed_property_new_value() const
	{
		return m_changed_property_new_value;
	}
}
