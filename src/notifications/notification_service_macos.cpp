#include "notifications/notification_service.h"
#include <filesystem>
#include <gio/gio.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include "system/environment.h"

using namespace desktop::system;

struct g_object_deleter
{
	void operator()(void* obj) const
	{
		if (obj)
		{
			g_object_unref(G_OBJECT(obj));
		}
	}
};

namespace desktop::notifications
{
	notification_service::notification_service(std::shared_ptr<app::app_info> app_info, std::shared_ptr<app::translation_service> translation_service)
	    : m_app_info{ std::move(app_info) },
	      m_translation_service{ std::move(translation_service) }
	{
	}

	notification_service::~notification_service() = default;

	const events::event<notification_service, app_notification_sent_event_args>& notification_service::get_app_notification_sent_event() const
	{
		return m_app_notification_sent_event;
	}

	void notification_service::send(const app_notification& notification)
	{
		m_app_notification_sent_event.invoke(*this, { notification });
	}

	void notification_service::send(const shell_notification& notification)
	{
		std::string icon_path{ m_app_info->get_id() + "-symbolic" };
		if (g_application_get_default())
		{
			std::unique_ptr<GNotification, g_object_deleter> gnotif{ g_notification_new(notification.get_title().c_str()) };
			std::unique_ptr<GIcon, g_object_deleter> icon{ g_themed_icon_new(icon_path.c_str()) };
			g_notification_set_body(gnotif.get(), notification.get_message().c_str());
			g_notification_set_icon(gnotif.get(), icon.get());
			if (notification.get_severity() == notification_severity::success)
			{
				g_notification_set_priority(gnotif.get(), G_NOTIFICATION_PRIORITY_HIGH);
			}
			else if (notification.get_severity() == notification_severity::warning || notification.get_severity() == notification_severity::error)
			{
				g_notification_set_priority(gnotif.get(), G_NOTIFICATION_PRIORITY_URGENT);
			}
			else
			{
				g_notification_set_priority(gnotif.get(), G_NOTIFICATION_PRIORITY_NORMAL);
			}
			if (notification.get_action() == "open" && std::filesystem::exists(notification.get_action_parameter()))
			{
				g_notification_add_button_with_target_value(gnotif.get(), m_translation_service->_("Open"), "app.open",
				                                            g_variant_new_string(notification.get_action_parameter().c_str()));
			}
			g_application_send_notification(g_application_get_default(), m_app_info->get_id().c_str(), gnotif.get());
		}
		else
		{
			Class nsUserNotification{ objc_getClass("NSUserNotification") };
			Class nsUserNotificationCenter{ objc_getClass("NSUserNotificationCenter") };
			Class nsString{ objc_getClass("NSString") };
			if (!nsUserNotification || !nsUserNotificationCenter || !nsString)
			{
				environment::execute("osascript -e 'display notification \"" + notification.get_message() + "\" with title \"" + notification.get_title() +
				                     "\" subtitle \"" + m_app_info->get_id() + "\"'");
				return;
			}
			id titleStr{ (reinterpret_cast<id (*)(id, SEL, const char*)>(objc_msgSend))(
				reinterpret_cast<id>(nsString), sel_registerName("stringWithUTF8String:"), notification.get_title().c_str()) };
			id messageStr{ (reinterpret_cast<id (*)(id, SEL, const char*)>(objc_msgSend))(
				reinterpret_cast<id>(nsString), sel_registerName("stringWithUTF8String:"), notification.get_message().c_str()) };
			id subtitleStr{ (reinterpret_cast<id (*)(id, SEL, const char*)>(objc_msgSend))(
				reinterpret_cast<id>(nsString), sel_registerName("stringWithUTF8String:"), m_app_info->get_id().c_str()) };
			id allocatedNotif{ (reinterpret_cast<id (*)(id, SEL)>(objc_msgSend))(reinterpret_cast<id>(nsUserNotification), sel_registerName("alloc")) };
			if (!allocatedNotif)
			{
				environment::execute("osascript -e 'display notification \"" + notification.get_message() + "\" with title \"" + notification.get_title() +
				                     "\" subtitle \"" + m_app_info->get_id() + "\"'");
				return;
			}
			id notif{ (reinterpret_cast<id (*)(id, SEL)>(objc_msgSend))(allocatedNotif, sel_registerName("init")) };
			if (!notif)
			{
				reinterpret_cast<void (*)(id, SEL)>(objc_msgSend)(allocatedNotif, sel_registerName("release"));
				environment::execute("osascript -e 'display notification \"" + notification.get_message() + "\" with title \"" + notification.get_title() +
				                     "\" subtitle \"" + m_app_info->get_id() + "\"'");
				return;
			}
			reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend)(notif, sel_registerName("setTitle:"), titleStr);
			reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend)(notif, sel_registerName("setInformativeText:"), messageStr);
			reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend)(notif, sel_registerName("setSubtitle:"), subtitleStr);
			id center{ (reinterpret_cast<id (*)(id, SEL)>(objc_msgSend))(reinterpret_cast<id>(nsUserNotificationCenter),
				                                                         sel_registerName("defaultUserNotificationCenter")) };
			if (!center)
			{
				reinterpret_cast<void (*)(id, SEL)>(objc_msgSend)(notif, sel_registerName("release"));
				environment::execute("osascript -e 'display notification \"" + notification.get_message() + "\" with title \"" + notification.get_title() +
				                     "\" subtitle \"" + m_app_info->get_id() + "\"'");
				return;
			}
			reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend)(center, sel_registerName("deliverNotification:"), notif);
			reinterpret_cast<void (*)(id, SEL)>(objc_msgSend)(notif, sel_registerName("release"));
		}
	}
}