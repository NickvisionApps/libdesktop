#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::hosting;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::system;

class Hosting_Test : public ::testing::Test
{
protected:
	host_options m_options{ nullptr, {} };
	host m_host{ m_options };
};

class test_lifetime_service : public lifetime_service
{
public:
	test_lifetime_service()
	    : lifetime_service{ std::make_shared<logger>(), false }
	{
	}

protected:
	void on_startup_and_run() override { }
	void on_shutdown() override { }
	void on_stop_requested() override { }
};

TEST_F(Hosting_Test, Host_servicesContainsDatabaseService)
{
	EXPECT_TRUE(m_host.get_services()->contains<database_service>());
}

TEST_F(Hosting_Test, Host_servicesContainsConfigurationService)
{
	EXPECT_TRUE(m_host.get_services()->contains<configuration_service>());
}

TEST_F(Hosting_Test, Host_servicesContainsNotificationService)
{
	EXPECT_TRUE(m_host.get_services()->contains<notification_service>());
}

TEST_F(Hosting_Test, Host_servicesContainsPowerService)
{
	EXPECT_TRUE(m_host.get_services()->contains<power_service>());
}

TEST_F(Hosting_Test, Host_servicesContainsSecretService)
{
	EXPECT_TRUE(m_host.get_services()->contains<secret_service>());
}

TEST_F(Hosting_Test, Host_servicesNotNull)
{
	EXPECT_NE(m_host.get_services(), nullptr);
}

TEST_F(Hosting_Test, HostOptions_defaultLogPath)
{
	host_options opts{ nullptr, {} };
	EXPECT_TRUE(opts.get_log_path().empty());
}

TEST_F(Hosting_Test, Host_servicesContainsAppInfo)
{
	std::shared_ptr<app_info> info{ std::make_shared<app_info>("com.example.test", "TestApp", "testapp") };
	host_options opts{ info, {} };
	host h{ opts };
	EXPECT_TRUE(h.get_services()->contains<app_info>());
}

TEST_F(Hosting_Test, HostOptions_getSetAppInfo)
{
	std::shared_ptr<app_info> info{ std::make_shared<app_info>("com.example.test", "TestApp", "testapp") };
	host_options opts{ info, {} };
	EXPECT_EQ(opts.get_app_info(), info);
}

TEST_F(Hosting_Test, HostOptions_getArgc)
{
	int argc{ 1 };
	char arg0[]{ "test" };
	char* argv[]{ arg0 };
	host_options opts{ nullptr, { argv, static_cast<std::size_t>(argc) } };
	EXPECT_EQ(opts.get_argc(), 1);
}

TEST_F(Hosting_Test, HostOptions_getArgv)
{
	int argc{ 1 };
	char arg0[]{ "test" };
	char* argv[]{ arg0 };
	host_options opts{ nullptr, { argv, static_cast<std::size_t>(argc) } };
	EXPECT_EQ(opts.get_argv(), argv);
}

TEST_F(Hosting_Test, HostOptions_setLogPath)
{
	host_options opts{ nullptr, {} };
	opts.set_log_path("test.log");
	EXPECT_EQ(opts.get_log_path(), "test.log");
}

TEST_F(Hosting_Test, LifetimeService_getStopSource)
{
	test_lifetime_service svc;
	EXPECT_NO_THROW(svc.get_stop_source());
}

TEST_F(Hosting_Test, LifetimeService_getUptime)
{
	test_lifetime_service svc;
	EXPECT_GE(svc.get_uptime().count(), 0);
}

TEST_F(Hosting_Test, LifetimeService_stop)
{
	test_lifetime_service svc;
	EXPECT_NO_THROW(svc.stop());
}
