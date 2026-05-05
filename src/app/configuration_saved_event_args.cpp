#include "app/configuration_saved_event_args.h"

namespace desktop::app
{
	configuration_saved_event_args::configuration_saved_event_args()
	    : m_changed_property_new_value{ "", "" },
	      m_is_bulk{ true }
	{
	}

	configuration_saved_event_args::configuration_saved_event_args(std::string changed_property_name, database_value changed_property_new_value)
	    : m_changed_property_name{ std::move(changed_property_name) },
	      m_changed_property_new_value{ std::move(changed_property_new_value) },
	      m_is_bulk{ false }
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

	bool configuration_saved_event_args::is_bulk() const
	{
		return m_is_bulk;
	}

	void configuration_saved_event_args::set_bulk(bool bulk)
	{
		m_is_bulk = bulk;
	}
}
