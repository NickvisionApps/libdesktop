#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::hosting;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::system;

class HostTest : public ::testing::Test
{
protected:
	int m_argc{ 0 };
	host_options m_options{ m_argc, nullptr };
	host m_host{ m_options };
};

TEST(HostingTest, HostOptions_StoresArgc)
{
	int argc{ 1 };
	char arg0[]{ "test" };
	char* argv[]{ arg0 };
	host_options opts{ argc, argv };
	EXPECT_EQ(opts.get_argc(), 1);
}

TEST(HostingTest, HostOptions_StoresArgv)
{
	int argc{ 1 };
	char arg0[]{ "test" };
	char* argv[]{ arg0 };
	host_options opts{ argc, argv };
	EXPECT_EQ(opts.get_argv(), argv);
}

TEST(HostingTest, HostOptions_DefaultLogPathEmpty)
{
	int argc{ 0 };
	host_options opts{ argc, nullptr };
	EXPECT_TRUE(opts.get_log_path().empty());
}

TEST(HostingTest, HostOptions_SetLogPath)
{
	int argc{ 0 };
	host_options opts{ argc, nullptr };
	opts.set_log_path("test.log");
	EXPECT_EQ(opts.get_log_path(), "test.log");
}

TEST_F(HostTest, Services_NotNull)
{
	EXPECT_NE(m_host.services(), nullptr);
}

TEST_F(HostTest, Services_ContainsNotificationService)
{
	EXPECT_TRUE(m_host.services()->contains<notification_service>());
}

TEST_F(HostTest, Services_ContainsSecretService)
{
	EXPECT_TRUE(m_host.services()->contains<secret_service>());
}

TEST_F(HostTest, Services_ContainsPowerService)
{
	EXPECT_TRUE(m_host.services()->contains<power_service>());
}
