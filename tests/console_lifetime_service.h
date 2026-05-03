#pragma once

#include <memory>
#include <tuple>
#include <libdesktop.h>

class console_lifetime_service : public desktop::hosting::lifetime_service
{
public:
	using dependencies = std::tuple<desktop::app::logger, desktop::app::arguments_service, desktop::notifications::notification_service, desktop::secrets::secret_service, desktop::system::power_service>;

	console_lifetime_service(std::shared_ptr<desktop::app::logger> logger, std::shared_ptr<desktop::app::arguments_service> argument_service, std::shared_ptr<desktop::notifications::notification_service> notification_service, std::shared_ptr<desktop::secrets::secret_service> secret_service, std::shared_ptr<desktop::system::power_service> power_service);

private:
	void on_startup_and_run() override;
	void on_shutdown() override;
	void on_stop_requested() override;
	bool m_running;
	std::shared_ptr<desktop::app::logger> m_logger;
	std::shared_ptr<desktop::app::arguments_service> m_arguments_service;
	std::shared_ptr<desktop::notifications::notification_service> m_notification_service;
	std::shared_ptr<desktop::secrets::secret_service> m_secret_service;
	std::shared_ptr<desktop::system::power_service> m_power_service;
};