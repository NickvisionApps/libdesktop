#pragma once

#include <chrono>
#include <memory>
#include <stop_token>
#include <thread>
#include "app/logger.h"
#include "services/service.h"

namespace desktop::hosting
{
	class lifetime_service : public services::service
	{
	public:
		lifetime_service(std::shared_ptr<app::logger> logger, bool graphical);
		~lifetime_service() override;
		lifetime_service(const lifetime_service&) = delete;
		lifetime_service(lifetime_service&&) = delete;
		const std::stop_source& get_stop_source() const;
		std::chrono::seconds get_uptime() const;
		void run();
		void stop();
		lifetime_service& operator=(const lifetime_service&) = delete;
		lifetime_service& operator=(lifetime_service&&) = delete;

	protected:
		virtual void on_startup_and_run() = 0;
		virtual void on_shutdown() = 0;
		virtual void on_stop_requested() = 0;

	private:
		void startup_and_run();
		void shutdown();
		std::shared_ptr<app::logger> m_logger;
		bool m_graphical;
		std::stop_source m_stop_source;
		std::thread m_thread;
		std::chrono::steady_clock::time_point m_start_time;
	};
}