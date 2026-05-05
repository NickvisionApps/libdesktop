#include "notifications/notification.h"

namespace desktop::notifications
{
	notification::notification(const std::string& message, notification_severity severity)
	    : m_message{ message },
	      m_severity{ severity }
	{
	}

	const std::string& notification::get_message() const
	{
		return m_message;
	}

	notification_severity notification::get_severity() const
	{
		return m_severity;
	}

	const std::string& notification::get_action() const
	{
		return m_action;
	}

	void notification::set_action(const std::string& action, const std::string& parameter)
	{
		m_action = action;
		m_action_parameter = parameter;
	}

	const std::string& notification::get_action_parameter() const
	{
		return m_action_parameter;
	}
}