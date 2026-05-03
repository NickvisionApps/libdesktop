#include "suite.h"
#include <iostream>

using namespace desktop::app;
using namespace desktop::events;
using namespace desktop::filesystem;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::system;

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

void suite::secrets(const std::shared_ptr<logger>& logger, const std::shared_ptr<secret_service>& secret_service)
{
	std::cout << "---Secrets Module---" << std::endl;
	const std::string name{ "libdesktop_test" };
	std::optional<secret> s{ secret_service->create(name) };
	if (!s)
	{
		throw std::runtime_error("Failed to create secret: " + name);
	}
	logger->info("Creating secret '" + name + "' with value: " + s->value(), __FILE__, __LINE__);
	std::optional<secret> fetched{ secret_service->get(name) };
	if (!fetched)
	{
		throw std::runtime_error("Failed to get secret: " + name);
	}
	if (fetched->value() != s->value())
	{
		throw std::runtime_error("Secret value mismatch after get.");
	}
	logger->info("Getting secret '" + name + "' with value: " + fetched->value(), __FILE__, __LINE__);
	secret updated{ name, "new_value" };
	if (!secret_service->update(updated))
	{
		throw std::runtime_error("Failed to update secret: " + name);
	}
	std::optional<secret> refetched{ secret_service->get(name) };
	if (!refetched || refetched->value() != "new_value")
	{
		throw std::runtime_error("Secret value mismatch after update.");
	}
	logger->info("Updating secret '" + name + "' with value: " + refetched->value(), __FILE__, __LINE__);
	if (!secret_service->remove(name))
	{
		throw std::runtime_error("Failed to remove secret: " + name);
	}
	std::optional<secret> removed{ secret_service->get(name) };
	if (removed)
	{
		throw std::runtime_error("Secret still exists after removal.");
	}
	logger->info("Removing secret '" + name + "'.", __FILE__, __LINE__);
}

void suite::system(const std::shared_ptr<logger>& logger, const std::shared_ptr<power_service>& power_service)
{
	std::cout << "---System Module---" << std::endl;
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello, World!" } };
#else
	process proc{ "/bin/echo", { "Hello, World!" } };
#endif
	if (!proc.start())
	{
		throw std::runtime_error("Failed to start process.");
	}
	int exit_code{ proc.wait_for_exit() };
	if (exit_code != 0)
	{
		throw std::runtime_error("Process exited with non-zero code: " + std::to_string(exit_code));
	}
	logger->info("Process '" + proc.get_path().string() + "' exited with code " + std::to_string(exit_code) + " and output: " + proc.get_standard_output(), __FILE__, __LINE__);
	logger->info("Is suspended: " + std::string(power_service->is_suspended() ? "true" : "false"), __FILE__, __LINE__);
	if (!power_service->prevent_suspend())
	{
		throw std::runtime_error("Failed to prevent suspend.");
	}
	if (!power_service->is_suspended())
	{
		throw std::runtime_error("Power service reports not suspended after prevent_suspend().");
	}
	logger->info("Suspend prevented. Is suspended: true", __FILE__, __LINE__);
	if (!power_service->allow_suspend())
	{
		throw std::runtime_error("Failed to allow suspend.");
	}
	if (power_service->is_suspended())
	{
		throw std::runtime_error("Power service reports suspended after allow_suspend().");
	}
	logger->info("Suspend allowed. Is suspended: false", __FILE__, __LINE__);
}

void suite::updates(const std::shared_ptr<logger>& logger)
{
	std::cout << "---Updates Module---" << std::endl;
}