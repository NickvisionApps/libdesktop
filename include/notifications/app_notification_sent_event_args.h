#pragma once

#include <chrono>
#include <memory>
#include "app_notification.h"
#include "events/event_args.h"

namespace desktop::notifications
{
	class app_notification_sent_event_args : public events::event_args
	{
	public:
		app_notification_sent_event_args(app_notification notification);
		~app_notification_sent_event_args() override = default;
		app_notification_sent_event_args(const app_notification_sent_event_args&) = default;
		app_notification_sent_event_args(app_notification_sent_event_args&&) noexcept = default;
		const app_notification& get_notification() const;
		const std::chrono::system_clock::time_point& get_timestamp() const;
		app_notification_sent_event_args& operator=(const app_notification_sent_event_args&) = default;
		app_notification_sent_event_args& operator=(app_notification_sent_event_args&&) noexcept = default;

	private:
		app_notification m_notification;
		std::chrono::system_clock::time_point m_timestamp;
	};
}