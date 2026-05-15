#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>
#include <tuple>
#include "app/app_info.h"

namespace desktop::hosting
{
	class lifetime_service
	{
	public:
		using dependencies = std::tuple<app::app_info>;
		lifetime_service(const std::shared_ptr<app::app_info>& info);
		virtual ~lifetime_service();
		lifetime_service(const lifetime_service&) = delete;
		lifetime_service(lifetime_service&&) = delete;
		std::chrono::seconds get_uptime() const;
		void request_restart() noexcept;
		void request_stop() noexcept;
		std::exception_ptr run();
		lifetime_service& operator=(const lifetime_service&) = delete;
		lifetime_service& operator=(lifetime_service&&) = delete;

	protected:
		virtual void on_startup_and_run() = 0;
		virtual void on_shutdown() noexcept = 0;
		virtual void on_stop_requested() noexcept = 0;

	private:
		void run_once();
		mutable std::mutex m_mutex;
		bool m_graphical;
		std::chrono::steady_clock::time_point m_start_time;
		std::stop_source m_stop_source;
		bool m_should_restart;
		std::thread m_worker;
		std::exception_ptr m_exception;
	};
}