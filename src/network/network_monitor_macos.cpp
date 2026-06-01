#include "network/network_monitor.h"
#include <Network/Network.h>
#include <condition_variable>
#include <dispatch/dispatch.h>
#include <stdexcept>

using namespace desktop::events;

namespace desktop::network
{
	class network_monitor::state
	{
	public:
		void handle_path_update(network_monitor& self, nw_path_t path);
		bool received_first_update{ false };
		std::condition_variable first_update_arrived;
		nw_path_monitor_t monitor{ nullptr };
		dispatch_queue_t queue{ nullptr };
	};

	network_monitor::network_monitor()
	    : m_state{ std::make_unique<state>() }
	{
		m_state->queue = dispatch_queue_create("libdesktop.network_monitor", DISPATCH_QUEUE_SERIAL);
		if (!m_state->queue)
		{
			throw std::runtime_error("Unable to create dispatch queue");
		}
		m_state->monitor = nw_path_monitor_create();
		if (!m_state->monitor)
		{
			dispatch_release(m_state->queue);
			m_state->queue = nullptr;
			throw std::runtime_error("Unable to create network path monitor");
		}
		nw_path_monitor_set_queue(m_state->monitor, m_state->queue);
		nw_path_monitor_set_update_handler(m_state->monitor, ^(nw_path_t path) { m_state->handle_path_update(*this, path); });
		nw_path_monitor_start(m_state->monitor);
		std::unique_lock lock{ m_mutex };
		m_state->first_update_arrived.wait(lock, [this]
		{
			return m_state->received_first_update;
		});
	}

	network_monitor::~network_monitor()
	{
		if (m_state->monitor)
		{
			nw_path_monitor_cancel(m_state->monitor);
			if (m_state->queue)
			{
				dispatch_sync(m_state->queue, ^{});
			}
			nw_release(m_state->monitor);
		}
		if (m_state->queue)
		{
			dispatch_release(m_state->queue);
		}
	}

	const event<network_monitor, param_event_args<network_state>>& network_monitor::get_state_changed_event() const
	{
		return m_state_changed_event;
	}

	network_state network_monitor::get_current_state() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_current_state;
	}

	void network_monitor::state::handle_path_update(network_monitor& self, nw_path_t path)
	{
		network_state new_state{ network_state::disconnected };
		switch (nw_path_get_status(path))
		{
		case nw_path_status_satisfied:
			new_state = network_state::connected_global;
			break;
		case nw_path_status_satisfiable:
			new_state = network_state::connected_local;
			break;
		case nw_path_status_unsatisfied:
		case nw_path_status_invalid:
		default:
			new_state = network_state::disconnected;
			break;
		}
		std::unique_lock lock{ self.m_mutex };
		bool first_update{ !received_first_update };
		received_first_update = true;
		bool changed{ self.m_current_state != new_state };
		if (changed)
		{
			self.m_current_state = new_state;
		}
		lock.unlock();
		if (first_update)
		{
			first_update_arrived.notify_all();
		}
		if (changed && !first_update)
		{
			self.m_state_changed_event.invoke(self, { new_state });
		}
	}
}