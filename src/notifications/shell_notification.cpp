#include "notifications/shell_notification.h"

namespace desktop::notifications
{
	shell_notification::shell_notification(const std::string& title, const std::string& message, notification_severity severity)
		: notification{ message, severity },
		m_title{ title }
	{

	}

	const std::string& shell_notification::get_title() const
	{
		return m_title;
	}
}