#pragma once

#include <mutex>
#include "events/event.h"
#include "services/service.h"
#include "notification_sent_event_args.h"

namespace desktop::notifications
{
	class notification_service : public services::service
	{
	public:
		notification_service() = default;
		const events::event<notification_service, notification_sent_event_args>& get_notification_sent_event() const;
		void send(const std::shared_ptr<notification>& notification);

	private:
		mutable std::mutex m_mutex;
		events::event<notification_service, notification_sent_event_args> m_notification_sent_event;
	};
}