#include "system/power_service.h"
#include <gio/gio.h>

namespace desktop::system
{
	power_service::power_service()
		: m_suspended{ false },
		m_cookie{ 0 }
	{

	}

	power_service::~power_service()
	{
		allow_suspend();
	}

	bool power_service::is_suspended() const
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		return m_suspended;
	}

	bool power_service::allow_suspend()
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		if(!m_suspended)
		{
			return true;
		}
		GError* error{ nullptr };
		GDBusProxy* proxy{ g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver", nullptr, &error) };
		if(error)
		{
			g_error_free(error);
			return false;
		}
		GVariant* result{ g_dbus_proxy_call_sync(proxy, "UnInhibit", g_variant_new("(u)", m_cookie), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error) };
		if(error)
		{
			g_error_free(error);
			g_object_unref(proxy);
			return false;
		}
		g_variant_unref(result);
		g_object_unref(proxy);
		m_cookie = 0;
		m_suspended = false;
		return true;
	}

	bool power_service::prevent_suspend()
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		if(m_suspended)
		{
			return true;
		}
		GError* error{ nullptr };
		GDBusProxy* proxy{ g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.freedesktop.ScreenSaver", "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver", nullptr, &error) };
		if(error)
		{
			g_error_free(error);
			return false;
		}
		GVariant* result{ g_dbus_proxy_call_sync(proxy, "Inhibit", g_variant_new("(ss)", "desktop", "Preventing suspend"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error) };
		if(error)
		{
			g_error_free(error);
			g_object_unref(proxy);
			return false;
		}
		g_variant_get(result, "(u)", &m_cookie);
		g_variant_unref(result);
		g_object_unref(proxy);
		m_suspended = true;
		return true;
	}
}
