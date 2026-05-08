#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::hosting;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::services;
using namespace desktop::system;

class test_base_service : public service
{
};

class test_derived_service : public test_base_service
{
};

class test_derived_value_service : public test_base_service
{
public:
	test_derived_value_service(std::string value)
	    : m_value{ std::move(value) }
	{
	}

	const std::string& get_value() const
	{
		return m_value;
	}

private:
	std::string m_value;
};

class test_value_service : public service
{
public:
	test_value_service(std::string value)
	    : m_value{ std::move(value) }
	{
	}

	const std::string& get_value() const
	{
		return m_value;
	}

private:
	std::string m_value;
};

class Services_Test : public ::testing::Test
{
protected:
	int m_argc{ 0 };
	host_options m_options{ nullptr, {} };
	host m_host{ m_options };
};

TEST_F(Services_Test, ServiceCollection_addService)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<notification_service>(service_scope::singleton, std::function<std::shared_ptr<notification_service>()>([]()
	{
		return std::make_shared<notification_service>();
	}));
	EXPECT_TRUE(sc->contains<notification_service>());
}

TEST_F(Services_Test, ServiceCollection_addServiceAutoResolve)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<notification_service>(service_scope::singleton);
	EXPECT_TRUE(sc->contains<notification_service>());
}

TEST_F(Services_Test, ServiceCollection_addServiceAutoResolveInterfaceImpl)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<test_base_service, test_derived_service>(service_scope::singleton);
	EXPECT_TRUE(sc->contains<test_base_service>());
}

TEST_F(Services_Test, ServiceCollection_addServiceInterfaceWithArgs)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<test_base_service, test_derived_value_service>(service_scope::singleton, std::string("hello"));
	std::shared_ptr<test_base_service> svc{ sc->get_service<test_base_service>() };
	ASSERT_NE(svc, nullptr);
	EXPECT_EQ(std::dynamic_pointer_cast<test_derived_value_service>(svc)->get_value(), "hello");
}

TEST_F(Services_Test, ServiceCollection_addServiceSingleTypeFactory)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<notification_service>(service_scope::singleton, std::function<std::shared_ptr<notification_service>()>([]()
	{
		return std::make_shared<notification_service>();
	}));
	EXPECT_TRUE(sc->contains<notification_service>());
}

TEST_F(Services_Test, ServiceCollection_addServiceWithArgs)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<test_value_service>(service_scope::singleton, std::string("hello"));
	std::shared_ptr<test_value_service> svc{ sc->get_service<test_value_service>() };
	ASSERT_NE(svc, nullptr);
	EXPECT_EQ(svc->get_value(), "hello");
}

TEST_F(Services_Test, ServiceCollection_getRequiredService)
{
	std::shared_ptr<notification_service> svc{ m_host.get_services()->get_required_service<notification_service>() };
	EXPECT_NE(svc, nullptr);
}

TEST_F(Services_Test, ServiceCollection_getRequiredServiceNotRegistered)
{
	EXPECT_THROW(m_host.get_services()->get_required_service<lifetime_service>(), std::runtime_error);
}

TEST_F(Services_Test, ServiceCollection_getService)
{
	std::shared_ptr<notification_service> svc{ m_host.get_services()->get_service<notification_service>() };
	EXPECT_NE(svc, nullptr);
}

TEST_F(Services_Test, ServiceCollection_getServiceNotRegistered)
{
	std::shared_ptr<lifetime_service> svc{ m_host.get_services()->get_service<lifetime_service>() };
	EXPECT_EQ(svc, nullptr);
}

TEST_F(Services_Test, ServiceCollection_removeService)
{
	m_host.get_services()->remove_service<notification_service>();
	EXPECT_FALSE(m_host.get_services()->contains<notification_service>());
}

TEST_F(Services_Test, ServiceCollection_transientScopeCreatesNewInstances)
{
	std::shared_ptr<service_collection> sc{ std::make_shared<service_collection>() };
	sc->add_service<notification_service>(service_scope::transient, std::function<std::shared_ptr<notification_service>()>([]()
	{
		return std::make_shared<notification_service>();
	}));
	std::shared_ptr<notification_service> first{ sc->get_service<notification_service>() };
	std::shared_ptr<notification_service> second{ sc->get_service<notification_service>() };
	EXPECT_NE(first, second);
}
