#include "notifications/notification_sent_event_args.h"

namespace desktop::notifications
{
	notification_sent_event_args::notification_sent_event_args(std::shared_ptr<notification> notification)
	    : m_notification{ std::move(notification) },
	      m_timestamp{ std::chrono::system_clock::now() }
	{
	}

	const std::shared_ptr<notification>& notification_sent_event_args::get_notification() const
	{
		return m_notification;
	}

	const std::chrono::system_clock::time_point& notification_sent_event_args::get_timestamp() const
	{
		return m_timestamp;
	}
}