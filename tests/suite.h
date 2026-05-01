#pragma once

#include <memory>
#include <libdesktop.h>

namespace suite
{
	void app(const std::shared_ptr<desktop::app::logger>& logger);
	void events(const std::shared_ptr<desktop::app::logger>& logger, const std::shared_ptr<desktop::notifications::notification_service>& notification_service);
	void filesystem(const std::shared_ptr<desktop::app::logger>& logger);
	void system(const std::shared_ptr<desktop::app::logger>& logger);
	void updates(const std::shared_ptr<desktop::app::logger>& logger);
}