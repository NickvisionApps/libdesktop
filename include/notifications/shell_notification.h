#pragma once

#include <string>
#include "notification.h"

namespace desktop::notifications
{
	class shell_notification : public notification
	{
	public:
		shell_notification(const std::string& title, const std::string& message, notification_severity severity);
		const std::string& get_title() const;

	private:
		std::string m_title;
	};
}