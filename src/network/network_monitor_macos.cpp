#include "network/network_monitor.h"
#include <Network/Network.h>
#include <condition_variable>
#include <dispatch/dispatch.h>
#include <stdexcept>

using namespace desktop::events;

namespace desktop::network
{
	class network_monitor::impl
	{
	public:
		impl(network_monitor& owner)
		    : m_owner{ owner },
		      m_queue{ dispatch_queue_create("libdesktop.network_monitor", DISPATCH_QUEUE_SERIAL) }
		{
			if (!m_queue)
			{
				throw std::runtime_error("Unable to create network monitor dispatch queue.");
			}
			m_monitor = nw_path_monitor_create();
			if (!m_monitor)
			{
				dispatch_release(m_queue);
				m_queue = nullptr;
				throw std::runtime_error("Unable to create network path monitor.");
			}
			nw_path_monitor_set_queue(m_monitor, m_queue);
			nw_path_monitor_set_update_handler(m_monitor, ^(nw_path_t path) { handle_path_update(path); });
			nw_path_monitor_start(m_monitor);
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_first_update_arrived.wait(lock, [this]
			{
				return m_received_first_update;
			});
		}

		~impl() noexcept
		{
			if (m_monitor)
			{
				nw_path_monitor_cancel(m_monitor);
				if (m_queue)
				{
					dispatch_sync(m_queue, ^{});
				}
				nw_release(m_monitor);
			}
			if (m_queue)
			{
				dispatch_release(m_queue);
			}
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
		void handle_path_update(nw_path_t path) noexcept
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
			std::unique_lock<std::mutex> lock{ m_mutex };
			bool first_update{ !m_received_first_update };
			m_received_first_update = true;
			bool changed{ m_current_state != new_state };
			if (changed)
			{
				m_current_state = new_state;
			}
			lock.unlock();
			if (first_update)
			{
				m_first_update_arrived.notify_all();
			}
			if (changed && !first_update)
			{
				m_owner.m_state_changed_event.invoke(m_owner, { new_state });
			}
		}

		mutable std::mutex m_mutex;
		std::condition_variable m_first_update_arrived;
		network_monitor& m_owner;
		network_state m_current_state{ network_state::disconnected };
		nw_path_monitor_t m_monitor{ nullptr };
		dispatch_queue_t m_queue{ nullptr };
		bool m_received_first_update{ false };
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