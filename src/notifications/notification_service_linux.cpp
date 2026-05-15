#include "notifications/notification_service.h"
#include <filesystem>
#include <gio/gio.h>
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

struct g_variant_deleter
{
	void operator()(GVariant* obj) const
	{
		if (obj)
		{
			g_variant_unref(obj);
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
		std::string icon_path;
		if (environment::has_variable("SNAP"))
		{
			icon_path = environment::get_variable("SNAP") + "/usr/share/icons/hicolor/symbolic/apps/" + m_app_info->get_id() + "-symbolic.svg";
		}
		else
		{
			icon_path = m_app_info->get_id() + "-symbolic";
		}
		if (g_application_get_default())
		{
			std::unique_ptr<GNotification, g_object_deleter> gnotif{ g_notification_new(notification.get_title().c_str()) };
			std::unique_ptr<GIcon, g_object_deleter> icon{ nullptr };
			std::unique_ptr<GFile, g_object_deleter> file_icon{ nullptr };
			if (!environment::has_variable("SNAP"))
			{
				icon.reset(g_themed_icon_new(icon_path.c_str()));
			}
			else
			{
				file_icon.reset(g_file_new_for_path(icon_path.c_str()));
				icon.reset(g_file_icon_new(file_icon.get()));
			}
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
			std::unique_ptr<GDBusConnection, g_object_deleter> connection{ g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, nullptr) };
			if (connection)
			{
				GVariant* params{ g_variant_new("(susssasa{sv}i)", m_app_info->get_id().c_str(), 0, icon_path.c_str(), notification.get_title().c_str(),
					                            notification.get_message().c_str(), nullptr, nullptr, -1) };
				std::unique_ptr<GVariant, g_variant_deleter> result{ g_dbus_connection_call_sync(
					connection.get(), "org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify", params,
					nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr) };
			}
		}
	}
}