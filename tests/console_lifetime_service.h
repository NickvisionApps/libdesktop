#pragma once

#include <memory>
#include <tuple>
#include <libdesktop.h>

class console_lifetime_service : public desktop::hosting::lifetime_service
{
public:
	using dependencies = std::tuple<desktop::app::logger, desktop::app::arguments_service>;

	console_lifetime_service(std::shared_ptr<desktop::app::logger> logger, std::shared_ptr<desktop::app::arguments_service> argument_service);

private:
	void on_startup_and_run() override;
	void on_shutdown() override;
	void on_stop_requested() override;
	bool m_running;
	std::shared_ptr<desktop::app::logger> m_logger;
	std::shared_ptr<desktop::app::arguments_service> m_arguments_service;
};