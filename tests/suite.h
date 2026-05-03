#pragma once

#include <memory>
#include <libdesktop.h>

namespace suite
{
	void app(const std::shared_ptr<desktop::app::logger>& logger);
	void events(const std::shared_ptr<desktop::app::logger>& logger, const std::shared_ptr<desktop::notifications::notification_service>& notification_service);
	void filesystem(const std::shared_ptr<desktop::app::logger>& logger);
	void secrets(const std::shared_ptr<desktop::app::logger>& logger, const std::shared_ptr<desktop::secrets::secret_service>& secret_service);
	void system(const std::shared_ptr<desktop::app::logger>& logger, const std::shared_ptr<desktop::system::power_service>& power_service);
	void updates(const std::shared_ptr<desktop::app::logger>& logger);
}