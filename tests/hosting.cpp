#include <chrono>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <stdexcept>
#include <thread>

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::hosting;
using namespace desktop::network;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::services;
using namespace desktop::system;
using namespace desktop::updates;

class console_lifetime_service : public lifetime_service
{
public:
	using dependencies = std::tuple<logger, app_info>;
	console_lifetime_service(std::shared_ptr<logger> logger, const std::shared_ptr<app_info>& info)
	    : lifetime_service{ info },
	      m_logger{ std::move(logger) }
	{
		m_logger->info("Starting", __FILE__, __LINE__);
	}
	~console_lifetime_service() override = default;

protected:
	void on_startup_and_run() override
	{
		m_logger->info("Running", __FILE__, __LINE__);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		m_logger->info("Finishing", __FILE__, __LINE__);
	}

	void on_shutdown() noexcept override
	{
		m_logger->info("Shutting down", __FILE__, __LINE__);
	}

	void on_stop_requested() noexcept override
	{
	}

private:
	std::shared_ptr<logger> m_logger;
};

class erroneous_lifetime_service : public lifetime_service
{
public:
	using dependencies = std::tuple<app_info>;
	erroneous_lifetime_service(const std::shared_ptr<app_info>& info)
	    : lifetime_service{ info }
	{
	}
	~erroneous_lifetime_service() override = default;

protected:
	void on_startup_and_run() override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		throw std::runtime_error{ "Test error" };
	}

	void on_shutdown() noexcept override
	{
	}

	void on_stop_requested() noexcept override
	{
	}
};

class restarting_lifetime_service : public lifetime_service
{
public:
	using dependencies = std::tuple<logger, app_info>;
	restarting_lifetime_service(std::shared_ptr<logger> logger, const std::shared_ptr<app_info>& info)
	    : lifetime_service{ info },
	      m_logger{ std::move(logger) }
	{
		m_logger->info("Starting", __FILE__, __LINE__);
	}
	~restarting_lifetime_service() override = default;

protected:
	void on_startup_and_run() override
	{
		m_logger->info("Running", __FILE__, __LINE__);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (!m_restarted)
		{
			m_logger->info("Restarting", __FILE__, __LINE__);
			request_restart();
			m_restarted = true;
		}
		m_logger->info("Finishing", __FILE__, __LINE__);
	}

	void on_shutdown() noexcept override
	{
		m_logger->info("Shutting down", __FILE__, __LINE__);
	}

	void on_stop_requested() noexcept override
	{
		m_logger->info("Stopping", __FILE__, __LINE__);
	}

private:
	std::shared_ptr<logger> m_logger;
	bool m_restarted{ false };
};

class stopping_lifetime_service : public lifetime_service
{
public:
	using dependencies = std::tuple<logger, app_info>;
	stopping_lifetime_service(std::shared_ptr<logger> logger, const std::shared_ptr<app_info>& info)
	    : lifetime_service{ info },
	      m_logger{ std::move(logger) }
	{
		m_logger->info("Starting", __FILE__, __LINE__);
	}
	~stopping_lifetime_service() override = default;

protected:
	void on_startup_and_run() override
	{
		m_logger->info("Running", __FILE__, __LINE__);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		request_stop();
		m_logger->info("Finishing", __FILE__, __LINE__);
	}

	void on_shutdown() noexcept override
	{
		m_logger->info("Shutting down", __FILE__, __LINE__);
	}

	void on_stop_requested() noexcept override
	{
		m_logger->info("Stopping", __FILE__, __LINE__);
	}

private:
	std::shared_ptr<logger> m_logger;
};

static const std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.hosting", "Test", "Test", false) };

TEST(Hosting, CapturesException)
{
	host host{ info, std::span<char*>{} };
	host.use_lifetime<erroneous_lifetime_service>();
	std::exception_ptr ptr;
	ASSERT_NO_THROW(ptr = host.run());
	ASSERT_TRUE(ptr);
}

TEST(Hosting, ContainsDefaults)
{
	host host{ info, std::span<char*>{} };
	ASSERT_TRUE(host.get_services()->get<app_info>());
	ASSERT_TRUE(host.get_services()->get<arguments_service>());
	ASSERT_TRUE(host.get_services()->get<configuration_service>());
	ASSERT_TRUE(host.get_services()->get<database_service>());
	ASSERT_TRUE(host.get_services()->get<http_service>());
	ASSERT_TRUE(host.get_services()->get<keyring_service>());
	ASSERT_TRUE(host.get_services()->get<logger>());
	ASSERT_TRUE(host.get_services()->get<notification_service>());
	ASSERT_TRUE(host.get_services()->get<power_service>());
	ASSERT_TRUE(host.get_services()->get<secret_service>());
	ASSERT_TRUE(host.get_services()->get<translation_service>());
	ASSERT_FALSE(host.get_services()->get<update_service>());
	ASSERT_FALSE(host.get_services()->get<lifetime_service>());
}

TEST(Hosting, Restarts)
{
	host host{ info, std::span<char*>{} };
	host.use_lifetime<restarting_lifetime_service>();
	std::exception_ptr ptr;
	ASSERT_NO_THROW(ptr = host.run());
	ASSERT_FALSE(ptr);
}

TEST(Hosting, Runs)
{
	host host{ info, std::span<char*>{} };
	host.get_services()->add<lifetime_service, console_lifetime_service>(service_scope::singleton);
	std::exception_ptr ptr;
	ASSERT_NO_THROW(ptr = host.run());
	ASSERT_FALSE(ptr);
}

TEST(Hosting, Stops)
{
	host host{ info, std::span<char*>{} };
	host.get_services()->add<lifetime_service, stopping_lifetime_service>(service_scope::singleton);
	std::exception_ptr ptr;
	ASSERT_NO_THROW(ptr = host.run());
	ASSERT_FALSE(ptr);
}