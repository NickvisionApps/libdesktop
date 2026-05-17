#include "network/network_monitor.h"
#include <gio/gio.h>
#include <stdexcept>

using namespace desktop::events;

static constexpr unsigned int NM_STATE_CONNECTED_LOCAL{ 50 };
static constexpr unsigned int NM_STATE_CONNECTED_SITE{ 60 };
static constexpr unsigned int NM_STATE_CONNECTED_GLOBAL{ 70 };

namespace desktop::network
{
	class network_monitor::impl
	{
	public:
		impl(network_monitor& owner)
		    : m_owner{ owner },
		      m_current_state{ network_state::disconnected },
		      m_proxy{ nullptr },
		      m_signal_handler_id{ 0 }
		{
			GError* error{ nullptr };
			m_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.freedesktop.NetworkManager",
			                                        "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", nullptr, &error);
			if (error)
			{
				g_error_free(error);
				throw std::runtime_error("Unable to create NetworkManager proxy.");
			}
			m_signal_handler_id = g_signal_connect_data(G_OBJECT(m_proxy), "g-properties-changed",
			                                            G_CALLBACK((void (*)(GDBusProxy*, GVariant*, GStrv, void*))(
			                                                [](GDBusProxy*, GVariant* changed_properties, GStrv, void* data)
			{
				GVariant* state{ g_variant_lookup_value(changed_properties, "State", G_VARIANT_TYPE_UINT32) };
				if (state)
				{
					g_variant_unref(state);
					static_cast<impl*>(data)->check_connection_state(true);
				}
			})),
			                                            this, nullptr, G_CONNECT_DEFAULT);
			if (m_signal_handler_id <= 0)
			{
				g_object_unref(m_proxy);
				throw std::runtime_error("Unable to connect to NetworkManager signal.");
			}
			check_connection_state(false);
		}

		~impl() noexcept
		{
			g_signal_handler_disconnect(G_OBJECT(m_proxy), m_signal_handler_id);
			g_object_unref(m_proxy);
		}

		impl(const impl&) = delete;

		impl(impl&&) noexcept = delete;

		network_state get_current_state() const
		{
			std::scoped_lock lock{ m_mutex };
			return m_current_state;
		}

		impl& operator=(const impl&) = delete;

		impl& operator=(impl&&) noexcept = delete;

	private:
		void check_connection_state(bool event) noexcept
		{
			network_state new_state{ network_state::disconnected };
			GVariant* result{ g_dbus_proxy_get_cached_property(m_proxy, "State") };
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
			std::unique_lock<std::mutex> lock{ m_mutex };
			if (m_current_state != new_state)
			{
				m_current_state = new_state;
				lock.unlock();
				if (event)
				{
					m_owner.m_state_changed_event.invoke(m_owner, { new_state });
				}
			}
		}
		mutable std::mutex m_mutex;
		network_monitor& m_owner;
		network_state m_current_state;
		GDBusProxy* m_proxy;
		unsigned long m_signal_handler_id;
	};

	network_monitor::network_monitor()
	    : m_impl{ std::make_unique<impl>(*this) }
	{
	}

	network_monitor::~network_monitor() = default;

	const event<network_monitor, param_event_args<network_state>>& network_monitor::get_state_changed_event() const
	{
		return m_state_changed_event;
	}

	network_state network_monitor::get_current_state() const
	{
		return m_impl->get_current_state();
	}
}