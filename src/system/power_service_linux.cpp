#include "system/power_service.h"
#include <gio/gio.h>

namespace desktop::system
{
	class power_service::state
	{
	public:
		GDBusProxy* proxy{ nullptr };
		unsigned int cookie{ 0 };
	};

	power_service::power_service()
	    : m_state{ std::make_unique<state>() }
	{
		GError* error{ nullptr };
		m_state->proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.freedesktop.ScreenSaver",
		                                               "/org/freedesktop/ScreenSaver", "org.freedesktop.ScreenSaver", nullptr, &error);
		if (error)
		{
			g_error_free(error);
		}
	}

	power_service::~power_service()
	{
		allow_suspend();
		if (m_state->proxy)
		{
			g_object_unref(m_state->proxy);
		}
	}

	bool power_service::is_suspended() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_suspended;
	}

	bool power_service::allow_suspend()
	{
		std::scoped_lock lock{ m_mutex };
		if (!m_state->proxy)
		{
			return false;
		}
		if (!m_suspended)
		{
			return true;
		}
		GError* error{ nullptr };
		GVariant* result{ g_dbus_proxy_call_sync(m_state->proxy, "UnInhibit", g_variant_new("(u)", m_state->cookie), G_DBUS_CALL_FLAGS_NONE, -1, nullptr,
			                                     &error) };
		if (error || !result)
		{
			if (error)
			{
				g_error_free(error);
			}
			return false;
		}
		g_variant_unref(result);
		m_state->cookie = 0;
		m_suspended = false;
		return true;
	}

	bool power_service::prevent_suspend()
	{
		std::scoped_lock lock{ m_mutex };
		if (!m_state->proxy)
		{
			return false;
		}
		if (m_suspended)
		{
			return true;
		}
		GError* error{ nullptr };
		GVariant* result{ g_dbus_proxy_call_sync(m_state->proxy, "Inhibit", g_variant_new("(ss)", "desktop", "Preventing suspend"), G_DBUS_CALL_FLAGS_NONE, -1,
			                                     nullptr, &error) };
		if (error || !result)
		{
			if (error)
			{
				g_error_free(error);
			}
			return false;
		}
		g_variant_get(result, "(u)", &m_state->cookie);
		g_variant_unref(result);
		if (m_state->cookie == 0)
		{
			return false;
		}
		m_suspended = true;
		return true;
	}
}
