#include "notifications/app_notification_sent_event_args.h"

namespace desktop::notifications
{
	app_notification_sent_event_args::app_notification_sent_event_args(app_notification notification)
	    : m_notification{ std::move(notification) },
	      m_timestamp{ std::chrono::system_clock::now() }
	{
	}

	const app_notification& app_notification_sent_event_args::get_notification() const
	{
		return m_notification;
	}

	const std::chrono::system_clock::time_point& app_notification_sent_event_args::get_timestamp() const
	{
		return m_timestamp;
	}
}