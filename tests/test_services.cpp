#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::hosting;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::services;
using namespace desktop::system;

class Services_Test : public ::testing::Test
{
protected:
	int m_argc{ 0 };
	host_options m_options{ m_argc, nullptr };
	host m_host{ m_options };
};

TEST_F(Services_Test, ServiceCollection_addService)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<notification_service, notification_service>(service_scope::singleton, std::function<std::shared_ptr<notification_service>()>([]() { return std::make_shared<notification_service>(); }));
	EXPECT_TRUE(sc->contains<notification_service>());
}

TEST_F(Services_Test, ServiceCollection_getRequiredService)
{
	std::shared_ptr<notification_service> svc{ m_host.services()->get_required_service<notification_service>() };
	EXPECT_NE(svc, nullptr);
}

TEST_F(Services_Test, ServiceCollection_getRequiredServiceNotRegistered)
{
	EXPECT_THROW(m_host.services()->get_required_service<lifetime_service>(), std::runtime_error);
}

TEST_F(Services_Test, ServiceCollection_getService)
{
	std::shared_ptr<notification_service> svc{ m_host.services()->get_service<notification_service>() };
	EXPECT_NE(svc, nullptr);
}

TEST_F(Services_Test, ServiceCollection_getServiceNotRegistered)
{
	std::shared_ptr<lifetime_service> svc{ m_host.services()->get_service<lifetime_service>() };
	EXPECT_EQ(svc, nullptr);
}

TEST_F(Services_Test, ServiceCollection_removeService)
{
	m_host.services()->remove_service<notification_service>();
	EXPECT_FALSE(m_host.services()->contains<notification_service>());
}
