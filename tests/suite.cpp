#include "suite.h"
#include <iostream>

using namespace desktop::app;
using namespace desktop::events;
using namespace desktop::filesystem;
using namespace desktop::notifications;

void suite::app(const std::shared_ptr<logger>& logger)
{
	std::cout << "---App Module---" << std::endl;
	logger->debug("This is a debug message.", __FILE__, __LINE__);
	logger->info("This is an info message.", __FILE__, __LINE__);
	logger->warn("This is a warning message.", __FILE__, __LINE__);
	logger->error("This is an error message.", __FILE__, __LINE__);
	logger->critical("This is a critical message.", __FILE__, __LINE__);
}

void suite::events(const std::shared_ptr<logger>& logger, const std::shared_ptr<notification_service>& notification_service)
{
	std::cout << "---Events Module---" << std::endl;
	int x = 0;
	logger->info("Registering 2 event handlers for notification sent event.", __FILE__, __LINE__);
	event_id id1 = notification_service->get_notification_sent_event().add_handler([&logger, &x](const desktop::notifications::notification_service&, const notification_sent_event_args&)
		{
			logger->debug("First notification sent handler invoked.", __FILE__, __LINE__);
			x++;
		});
	event_id id2 = notification_service->get_notification_sent_event().add_handler([&logger, &x](const desktop::notifications::notification_service&, const notification_sent_event_args&)
		{
			logger->debug("Second notification sent handler invoked.", __FILE__, __LINE__);
			x++;
		});
	logger->info("Sent test notification.", __FILE__, __LINE__);
	notification_service->send(std::make_shared<notification>("Test", notification_severity::information));
	if (x != 2)
	{
		throw std::runtime_error("Received " + std::to_string(x) + " event handler invocations. Expected 2.");
	}
	logger->info("Received 2 event handler invocations.", __FILE__, __LINE__);
	notification_service->get_notification_sent_event().remove_handler(id1);
	notification_service->get_notification_sent_event().remove_handler(id2);
}

void suite::filesystem(const std::shared_ptr<logger>& logger)
{
	std::cout << "---Filesystem Module---" << std::endl;
	logger->info("Cache      : " + user_directories::get_cache().string(), __FILE__, __LINE__);
	logger->info("Config     : " + user_directories::get_config().string(), __FILE__, __LINE__);
	logger->info("Desktop    : " + user_directories::get_desktop().string(), __FILE__, __LINE__);
	logger->info("Documents  : " + user_directories::get_documents().string(), __FILE__, __LINE__);
	logger->info("Downloads  : " + user_directories::get_downloads().string(), __FILE__, __LINE__);
	logger->info("Home       : " + user_directories::get_home().string(), __FILE__, __LINE__);
	logger->info("Local Data : " + user_directories::get_local_data().string(), __FILE__, __LINE__);
	logger->info("Music      : " + user_directories::get_music().string(), __FILE__, __LINE__);
	logger->info("Pictures   : " + user_directories::get_pictures().string(), __FILE__, __LINE__);
	logger->info("Templates  : " + user_directories::get_templates().string(), __FILE__, __LINE__);
	logger->info("Videos     : " + user_directories::get_videos().string(), __FILE__, __LINE__);
}

void suite::system(const std::shared_ptr<logger>& logger)
{
	std::cout << "---System Module---" << std::endl;
}

void suite::updates(const std::shared_ptr<logger>& logger)
{
	std::cout << "---Updates Module---" << std::endl;
}