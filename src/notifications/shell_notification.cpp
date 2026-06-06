#include "notifications/shell_notification.h"

namespace desktop::notifications
{
	shell_notification::shell_notification(std::string title, std::string message, notification_severity severity, std::string action,
	                                       std::string action_parameter)
	    : app_notification{ std::move(message), severity, std::move(action), std::move(action_parameter) },
	      m_title{ std::move(title) }
	{
	}

	const std::string& shell_notification::get_title() const
	{
		return m_title;
	}
}