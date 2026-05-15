#include "notifications/notification_service.h"

namespace desktop::notifications
{
	notification_service::notification_service(std::shared_ptr<app::app_info> app_info, std::shared_ptr<app::translation_service> translation_service)
	    : m_app_info{ std::move(app_info) },
	      m_translation_service{ std::move(translation_service) }
	{
	}

	const events::event<notification_service, notification_sent_event_args>& notification_service::get_notification_sent_event() const
	{
		return m_notification_sent_event;
	}

	void notification_service::send(const notification& notification)
	{
		m_notification_sent_event.invoke(*this, { notification });
	}

	void notification_service::send(const shell_notification& notification)
	{
		return;
	}
}