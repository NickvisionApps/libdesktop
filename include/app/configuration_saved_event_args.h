#pragma once

#include <string>
#include "app/database_value.h"
#include "events/event_args.h"

namespace desktop::app
{
	class configuration_saved_event_args : public events::event_args
	{
	public:
		configuration_saved_event_args();
		configuration_saved_event_args(const std::string& changed_property_name, const database_value& changed_property_new_value);
		~configuration_saved_event_args() override = default;
		configuration_saved_event_args(const configuration_saved_event_args&) = default;
		configuration_saved_event_args(configuration_saved_event_args&&) noexcept = default;
		const std::string& get_changed_property_name() const;
		const database_value& get_changed_property_new_value() const;
		bool is_bulk() const;
		void set_bulk(bool bulk);
		configuration_saved_event_args& operator=(const configuration_saved_event_args&) = default;
		configuration_saved_event_args& operator=(configuration_saved_event_args&&) noexcept = default;

	private:
		std::string m_changed_property_name;
		database_value m_changed_property_new_value;
		bool m_is_bulk;
	};
}
