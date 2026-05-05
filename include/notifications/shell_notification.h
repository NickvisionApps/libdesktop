#pragma once

#include <string>
#include "notification.h"

namespace desktop::notifications
{
	class shell_notification : public notification
	{
	public:
		shell_notification(std::string title, std::string message, notification_severity severity);
		~shell_notification() override = default;
		shell_notification(const shell_notification&) = default;
		shell_notification(shell_notification&&) noexcept = default;
		const std::string& get_title() const;
		shell_notification& operator=(const shell_notification&) = default;
		shell_notification& operator=(shell_notification&&) noexcept = default;

	private:
		std::string m_title;
	};
}