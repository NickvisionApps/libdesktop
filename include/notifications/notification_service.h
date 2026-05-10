#pragma once

#include "events/event.h"
#include "notification_sent_event_args.h"

namespace desktop::notifications
{
	class notification_service
	{
	public:
		notification_service() = default;
		~notification_service() = default;
		notification_service(const notification_service&) = delete;
		notification_service(notification_service&&) = delete;
		const events::event<notification_service, notification_sent_event_args>& get_notification_sent_event() const;
		void send(const std::shared_ptr<notification>& notification);
		notification_service& operator=(const notification_service&) = delete;
		notification_service& operator=(notification_service&&) = delete;

	private:
		events::event<notification_service, notification_sent_event_args> m_notification_sent_event;
	};
}