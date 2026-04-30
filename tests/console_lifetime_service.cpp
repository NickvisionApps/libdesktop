#include "console_lifetime_service.h"
#include <iostream>
#include <stdexcept>
#include <string>

using namespace desktop::app;
using namespace desktop::events;
using namespace desktop::filesystem;
using namespace desktop::hosting;
using namespace desktop::notifications;
using namespace desktop::services;

console_lifetime_service::console_lifetime_service(std::shared_ptr<logger> logger, std::shared_ptr<arguments_service> argument_service, std::shared_ptr<notification_service> notification_service)
	: lifetime_service{ logger, false },
	m_running{ false },
	m_logger{ logger },
	m_arguments_service{ argument_service },
	m_notification_service{ notification_service }
{

}

void console_lifetime_service::on_startup_and_run()
{
	m_logger->debug("Starting console.", __FILE__, __LINE__);
	m_running = true;
	std::cout << "===libdesktop Testing Program===" << std::endl;
	for (const std::string& argument : m_arguments_service->get_all())
	{
		std::cout << "Received argument: " << argument << std::endl;
	}
	do
	{
		std::cout << "\nSelect a module to test:" << std::endl;
		std::cout << "1. App" << std::endl;
		std::cout << "2. Events" << std::endl;
		std::cout << "3. Filesystem" << std::endl;
		std::cout << "4. System" << std::endl;
		std::cout << "5. All Modules" << std::endl;
		std::cout << "6. Exit" << std::endl;
		std::cout << "Option > ";
		std::string res;
		std::cin >> res;
		m_logger->debug("Received user option: " + res, __FILE__, __LINE__);
		try
		{
			switch (std::stoi(res))
			{
			case 1:
			{
				std::cout << "---App Module---" << std::endl;
				m_logger->debug("This is a debug message.", __FILE__, __LINE__);
				m_logger->info("This is an info message.", __FILE__, __LINE__);
				m_logger->warn("This is a warning message.", __FILE__, __LINE__);
				m_logger->error("This is an error message.", __FILE__, __LINE__);
				m_logger->critical("This is a critical message.", __FILE__, __LINE__);
				break;
			}
			case 2:
			{
				std::cout << "---Events Module---" << std::endl;
				int x = 0;
				m_logger->info("Registering 2 event handlers for notification sent event.", __FILE__, __LINE__);
				event_id id1 = m_notification_service->get_notification_sent_event().add_handler([this, &x](const notification_service&, const notification_sent_event_args&)
				{
					m_logger->debug("First notification sent handler invoked.", __FILE__, __LINE__);
					x++;
				});
				event_id id2 = m_notification_service->get_notification_sent_event().add_handler([this, &x](const notification_service&, const notification_sent_event_args&)
				{
					m_logger->debug("Second notification sent handler invoked.", __FILE__, __LINE__);
					x++;
				});
				m_logger->info("Sent test notification.", __FILE__, __LINE__);
				m_notification_service->send(std::make_shared<notification>("Test", notification_severity::information));
				if (x != 2)
				{
					throw std::runtime_error("Received " + std::to_string(x) + " event handler invocations. Expected 2.");
				}
				m_logger->info("Received 2 event handler invocations.", __FILE__, __LINE__);
				m_notification_service->get_notification_sent_event().remove_handler(id1);
				m_notification_service->get_notification_sent_event().remove_handler(id2);
				break;
			}
			case 3:
			{
				std::cout << "---Filesystem Module---" << std::endl;
				m_logger->info("Cache      : " + user_directories::get_cache().string(), __FILE__, __LINE__);
				m_logger->info("Config     : " + user_directories::get_config().string(), __FILE__, __LINE__);
				m_logger->info("Desktop    : " + user_directories::get_desktop().string(), __FILE__, __LINE__);
				m_logger->info("Documents  : " + user_directories::get_documents().string(), __FILE__, __LINE__);
				m_logger->info("Downloads  : " + user_directories::get_downloads().string(), __FILE__, __LINE__);
				m_logger->info("Home       : " + user_directories::get_home().string(), __FILE__, __LINE__);
				m_logger->info("Local Data : " + user_directories::get_local_data().string(), __FILE__, __LINE__);
				m_logger->info("Music      : " + user_directories::get_music().string(), __FILE__, __LINE__);
				m_logger->info("Pictures   : " + user_directories::get_pictures().string(), __FILE__, __LINE__);
				m_logger->info("Templates  : " + user_directories::get_templates().string(), __FILE__, __LINE__);
				m_logger->info("Videos     : " + user_directories::get_videos().string(), __FILE__, __LINE__);
				break;
			}
			case 4:
			{
				std::cout << "---System Module---" << std::endl;
				break;
			}
			case 5:
			{
				std::cout << "---All Modules---" << std::endl;
				break;
			}
			case 6:
			{
				stop();
				break;
			}
			default:
			{
				throw std::invalid_argument(res);
			}
			}
		}
		catch (const std::runtime_error& r)
		{
			m_logger->error(r.what(), __FILE__, __LINE__);
		}
		catch (...)
		{
			m_logger->warn("Invalid option selected: " + res, __FILE__, __LINE__);
		}

	} while (m_running);
}

void console_lifetime_service::on_shutdown()
{
	m_logger->debug("Shutting down console.", __FILE__, __LINE__);
	std::cout << "\nGoodbye!" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

void console_lifetime_service::on_stop_requested()
{
	m_logger->debug("Stopping console.", __FILE__, __LINE__);
	m_running = false;
}	