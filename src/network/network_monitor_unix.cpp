#include "network/network_monitor.h"
#include <gio/gio.h>
#include <stdexcept>

using namespace desktop::events;

namespace desktop::network
{
	class network_monitor::impl
	{
	public:
		impl(network_monitor& owner)
		    : m_owner{ owner },
		      m_current_state{ network_state::disconnected },
		      m_signal_handler_id{ 0 }
		{
			m_signal_handler_id = g_signal_connect_data(G_OBJECT(g_network_monitor_get_default()), "network-changed",
			                                            G_CALLBACK((void (*)(GNetworkMonitor*, gboolean, void*))(
			                                                [](GNetworkMonitor*, gboolean, void* data)
			{
				static_cast<impl*>(data)->check_connection_state(true);
			})),
			                                            this, nullptr, G_CONNECT_DEFAULT);
			if (m_signal_handler_id <= 0)
			{
				throw std::runtime_error("Unable to connect to network monitor signal.");
			}
			check_connection_state(false);
		}

		~impl() noexcept
		{
			g_signal_handler_disconnect(G_OBJECT(g_network_monitor_get_default()), m_signal_handler_id);
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
			GNetworkConnectivity connectivity{ g_network_monitor_get_connectivity(g_network_monitor_get_default()) };
			if (connectivity == G_NETWORK_CONNECTIVITY_FULL)
			{
				new_state = network_state::connected_global;
			}
			else if (connectivity == G_NETWORK_CONNECTIVITY_LOCAL)
			{
				new_state = network_state::disconnected;
			}
			else
			{
				new_state = network_state::connected_local;
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
		gulong m_signal_handler_id;
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