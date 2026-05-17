#include "notifications/app_notification.h"

namespace desktop::notifications
{
	app_notification::app_notification(std::string message, notification_severity severity)
	    : m_message{ std::move(message) },
	      m_severity{ severity }
	{
	}

	const std::string& app_notification::get_message() const
	{
		return m_message;
	}

	notification_severity app_notification::get_severity() const
	{
		return m_severity;
	}

	const std::string& app_notification::get_action() const
	{
		return m_action;
	}

	void app_notification::set_action(const std::string& action, const std::string& parameter)
	{
		m_action = action;
		m_action_parameter = parameter;
	}

	const std::string& app_notification::get_action_parameter() const
	{
		return m_action_parameter;
	}
}