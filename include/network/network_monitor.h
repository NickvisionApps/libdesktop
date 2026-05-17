#pragma once

#include <memory>
#include <mutex>
#include "events/event.h"
#include "events/param_event_args.h"
#include "network_state.h"

namespace desktop::network
{
	class network_monitor
	{
	public:
		network_monitor();
		~network_monitor();
		network_monitor(const network_monitor&) = delete;
		network_monitor(network_monitor&&) noexcept = delete;
		const events::event<network_monitor, events::param_event_args<network_state>>& get_state_changed_event() const;
		network_state get_current_state() const;
		network_monitor& operator=(const network_monitor&) = delete;
		network_monitor& operator=(network_monitor&&) noexcept = delete;

	private:
		class impl;
		friend class impl;
		std::unique_ptr<impl> m_impl;
		events::event<network_monitor, events::param_event_args<network_state>> m_state_changed_event;
	};
}