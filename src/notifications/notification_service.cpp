#include "notifications/notification_service.h"

namespace desktop::notifications
{
	const events::event<notification_service, notification_sent_event_args>& notification_service::get_notification_sent_event() const
	{
		return m_notification_sent_event;
	}

	void notification_service::send(const std::shared_ptr<notification>& notification)
	{
		m_notification_sent_event.invoke(*this, { notification });
	}
}