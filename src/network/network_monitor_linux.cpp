#include "network/network_monitor.h"
#include <gio/gio.h>
#include <stdexcept>

using namespace desktop::events;

static constexpr unsigned int NM_STATE_CONNECTED_LOCAL{ 50 };
static constexpr unsigned int NM_STATE_CONNECTED_SITE{ 60 };
static constexpr unsigned int NM_STATE_CONNECTED_GLOBAL{ 70 };

namespace desktop::network
{
	class network_monitor::state
	{
	public:
		void check_connection_state(network_monitor& self, bool event);
		GDBusProxy* proxy{ nullptr };
		unsigned long signal_handler_id{ 0 };
	};

	network_monitor::network_monitor()
	    : m_state{ std::make_unique<state>() }
	{
		GError* error{ nullptr };
		m_state->proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.freedesktop.NetworkManager",
		                                               "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", nullptr, &error);
		if (error)
		{
			g_error_free(error);
			throw std::runtime_error("Unable to create NetworkManager proxy");
		}
		m_state->signal_handler_id = g_signal_connect_data(G_OBJECT(m_state->proxy), "g-properties-changed",
		                                                   G_CALLBACK((void (*)(GDBusProxy*, GVariant*, GStrv, void*))(
		                                                       [](GDBusProxy*, GVariant* changed_properties, GStrv, void* data)
		{
			GVariant* state_var{ g_variant_lookup_value(changed_properties, "State", G_VARIANT_TYPE_UINT32) };
			if (state_var)
			{
				g_variant_unref(state_var);
				network_monitor* monitor{ static_cast<network_monitor*>(data) };
				monitor->m_state->check_connection_state(*monitor, true);
			}
		})),
		                                                   this, nullptr, G_CONNECT_DEFAULT);
		if (m_state->signal_handler_id <= 0)
		{
			g_object_unref(m_state->proxy);
			throw std::runtime_error("Unable to connect to NetworkManager signal");
		}
		m_state->check_connection_state(*this, false);
	}

	network_monitor::~network_monitor()
	{
		g_signal_handler_disconnect(G_OBJECT(m_state->proxy), m_state->signal_handler_id);
		g_object_unref(m_state->proxy);
	}

	const event<network_monitor, param_event_args<network_state>>& network_monitor::get_state_changed_event() const
	{
		return m_state_changed_event;
	}

	network_state network_monitor::get_current_state() const
	{
		return m_current_state;
	}

	void network_monitor::state::check_connection_state(network_monitor& self, bool event)
	{
		network_state new_state{ network_state::disconnected };
		GVariant* result{ g_dbus_proxy_get_cached_property(proxy, "State") };
		if (result)
		{
			guint32 state{ g_variant_get_uint32(result) };
			g_variant_unref(result);
			if (state == NM_STATE_CONNECTED_GLOBAL)
			{
				new_state = network_state::connected_global;
			}
			else if (state == NM_STATE_CONNECTED_SITE || state == NM_STATE_CONNECTED_LOCAL)
			{
				new_state = network_state::connected_local;
			}
		}
		std::unique_lock<std::mutex> lock{ self.m_mutex };
		if (self.m_current_state != new_state)
		{
			self.m_current_state = new_state;
			lock.unlock();
			if (event)
			{
				self.m_state_changed_event.invoke(self, { new_state });
			}
		}
	}
}