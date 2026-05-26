#pragma once

#include <memory>
#include <tuple>
#include "app_info.h"
#include "events/event.h"
#include "events/param_event_args.h"

namespace desktop::app
{
	class ipc_service
	{
	public:
		using dependencies = std::tuple<app_info>;
		ipc_service(std::shared_ptr<app_info> app_info);
		~ipc_service() = default;
		ipc_service(const ipc_service&) = delete;
		ipc_service(ipc_service&&) = delete;
		const events::event<ipc_service, events::param_event_args<std::string>>& get_message_received_event() const;
		bool is_host() const;
		bool send_message(const std::string& message);
		ipc_service& operator=(const ipc_service&) = delete;
		ipc_service& operator=(ipc_service&&) = delete;

	private:
		class impl;
		friend class impl;
		std::shared_ptr<app_info> m_app_info;
		std::unique_ptr<impl> m_impl;
		events::event<ipc_service, events::param_event_args<std::string>> m_message_received_event;
	};
}