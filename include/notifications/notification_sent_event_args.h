#pragma once

#include <chrono>
#include <memory>
#include <string>
#include "events/event_args.h"
#include "notification.h"

namespace desktop::notifications
{
	class notification_sent_event_args : public events::event_args
	{
	public:
		notification_sent_event_args(std::shared_ptr<notification> notification);
		std::shared_ptr<notification> get_notification() const;
		const std::chrono::system_clock::time_point& get_timestamp() const;

	private:
		std::shared_ptr<notification> m_notification;
		std::chrono::system_clock::time_point m_timestamp;
	};
}