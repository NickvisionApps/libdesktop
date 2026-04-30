#include "console_lifetime_service.h"
#include <iostream>
#include <stdexcept>
#include <string>

using namespace desktop::app;
using namespace desktop::hosting;
using namespace desktop::services;

console_lifetime_service::console_lifetime_service(std::shared_ptr<logger> logger, std::shared_ptr<arguments_service> argument_service)
	: lifetime_service{ logger, false },
	m_running{ false },
	m_logger{ logger },
	m_arguments_service{ argument_service }
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
		std::cout << "3. Services" << std::endl;
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
				std::cout << "---App Module---" << std::endl;
				m_logger->debug("This is a debug message.", __FILE__, __LINE__);
				m_logger->info("This is an info message.", __FILE__, __LINE__);
				m_logger->warn("This is a warning message.", __FILE__, __LINE__);
				m_logger->error("This is an error message.", __FILE__, __LINE__);
				m_logger->critical("This is a critical message.", __FILE__, __LINE__);
				std::cout << "---Success---" << std::endl;
				break;
			case 2:
				std::cout << "---Events Module---" << std::endl;
				std::cout << "---Success---" << std::endl;
				break;
			case 3:
				std::cout << "---Services Module---" << std::endl;
				std::cout << "---Success---" << std::endl;
				break;
			case 4:
				std::cout << "---System Module---" << std::endl;
				std::cout << "---Success---" << std::endl;
				break;
			case 5:
				std::cout << "---All Modules---" << std::endl;
				std::cout << "---Success---" << std::endl;
				break;
			case 6:
				stop();
				break;
			default:
				throw std::exception();
			}
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