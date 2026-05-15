#pragma once

#include <memory>
#include <tuple>
#include "app/app_info.h"
#include "app/translation_service.h"
#include "events/event.h"
#include "notification.h"
#include "notification_sent_event_args.h"
#include "shell_notification.h"

namespace desktop::notifications
{
	class notification_service
	{
	public:
		using dependencies = std::tuple<app::app_info, app::translation_service>;
		notification_service(std::shared_ptr<app::app_info> app_info, std::shared_ptr<app::translation_service> translation_service);
		~notification_service() = default;
		notification_service(const notification_service&) = delete;
		notification_service(notification_service&&) = delete;
		const events::event<notification_service, notification_sent_event_args>& get_notification_sent_event() const;
		void send(const notification& notification);
		void send(const shell_notification& notification);
		notification_service& operator=(const notification_service&) = delete;
		notification_service& operator=(notification_service&&) = delete;

	private:
		std::shared_ptr<app::app_info> m_app_info;
		std::shared_ptr<app::translation_service> m_translation_service;
		events::event<notification_service, notification_sent_event_args> m_notification_sent_event;
	};
}