#include "console_lifetime_service.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include "suite.h"

using namespace desktop::app;
using namespace desktop::hosting;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::services;
using namespace desktop::system;

console_lifetime_service::console_lifetime_service(std::shared_ptr<logger> logger, std::shared_ptr<arguments_service> argument_service, std::shared_ptr<notification_service> notification_service, std::shared_ptr<secret_service> secret_service, std::shared_ptr<power_service> power_service)
	: lifetime_service{ logger, false },
	m_running{ false },
	m_logger{ logger },
	m_arguments_service{ argument_service },
	m_notification_service{ notification_service },
	m_secret_service{ secret_service },
	m_power_service{ power_service }
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
		std::cout << "4. Secrets" << std::endl;
		std::cout << "5. System" << std::endl;
		std::cout << "6. Updates" << std::endl;
		std::cout << "7. All Modules" << std::endl;
		std::cout << "8. Exit" << std::endl;
		std::cout << "Option > ";
		std::string res;
		std::cin >> res;
		m_logger->debug("Received user option: " + res, __FILE__, __LINE__);
		try
		{
			switch (std::stoi(res))
			{
			case 1:
				suite::app(m_logger);
				break;
			case 2:
				suite::events(m_logger, m_notification_service);
				break;
			case 3:
				suite::filesystem(m_logger);
				break;
			case 4:
				suite::secrets(m_logger, m_secret_service);
				break;
			case 5:
				suite::system(m_logger, m_power_service);
				break;
			case 6:
				suite::updates(m_logger);
				break;
			case 7:
				suite::app(m_logger);
				suite::events(m_logger, m_notification_service);
				suite::filesystem(m_logger);
				suite::secrets(m_logger, m_secret_service);
				suite::system(m_logger, m_power_service);
				suite::updates(m_logger);
				break;
			case 8:
				stop();
				break;
			default:
				throw std::invalid_argument(res);
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