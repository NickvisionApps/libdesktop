#pragma once

#include <chrono>
#include <memory>
#include "events/event_args.h"
#include "notification.h"

namespace desktop::notifications
{
	class notification_sent_event_args : public events::event_args
	{
	public:
		notification_sent_event_args(std::shared_ptr<notification> notification);
		~notification_sent_event_args() override = default;
		notification_sent_event_args(const notification_sent_event_args&) = default;
		notification_sent_event_args(notification_sent_event_args&&) noexcept = default;
		const std::shared_ptr<notification>& get_notification() const;
		const std::chrono::system_clock::time_point& get_timestamp() const;
		notification_sent_event_args& operator=(const notification_sent_event_args&) = default;
		notification_sent_event_args& operator=(notification_sent_event_args&&) noexcept = default;

	private:
		std::shared_ptr<notification> m_notification;
		std::chrono::system_clock::time_point m_timestamp;
	};
}