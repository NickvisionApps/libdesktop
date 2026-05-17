#pragma once

#include <string>
#include "notification_severity.h"

namespace desktop::notifications
{
	class app_notification
	{
	public:
		app_notification(std::string message, notification_severity severity);
		virtual ~app_notification() = default;
		app_notification(const app_notification&) = default;
		app_notification(app_notification&&) noexcept = default;
		const std::string& get_message() const;
		notification_severity get_severity() const;
		const std::string& get_action() const;
		void set_action(const std::string& action, const std::string& parameter = "");
		const std::string& get_action_parameter() const;
		app_notification& operator=(const app_notification&) = default;
		app_notification& operator=(app_notification&&) noexcept = default;

	private:
		std::string m_message;
		notification_severity m_severity;
		std::string m_action;
		std::string m_action_parameter;
	};
}