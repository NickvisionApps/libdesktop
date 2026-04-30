#pragma once

#include <string>
#include "notification_severity.h"

namespace desktop::notifications
{
	class notification
	{
	public:
		notification(const std::string& message, notification_severity severity);
		const std::string& get_message() const;
		notification_severity get_severity() const;
		const std::string& get_action() const;
		void set_action(const std::string& action, const std::string& parameter = "");
		const std::string& get_action_parameter() const;
			
	private:
		std::string m_message;
		notification_severity m_severity;
		std::string m_action;
		std::string m_action_parameter;
	};
}