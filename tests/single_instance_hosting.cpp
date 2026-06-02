#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <memory>
#include <stdexcept>
#include <thread>

using namespace desktop::app;
using namespace desktop::hosting;
using namespace desktop::services;

class gated_lifetime_service : public lifetime_service
{
public:
	using dependencies = std::tuple<app_info>;
	gated_lifetime_service(const std::shared_ptr<app_info>& info)
	    : lifetime_service{ info }
	{
	}
	~gated_lifetime_service() override = default;
	std::future<void> get_started_future()
	{
		return m_started.get_future();
	}
	void release()
	{
		m_release.set_value();
	}

protected:
	void on_startup_and_run() override
	{
		m_started.set_value();
		m_release.get_future().wait();
	}
	void on_shutdown() noexcept override
	{
	}
	void on_stop_requested() noexcept override
	{
	}

private:
	std::promise<void> m_started;
	std::promise<void> m_release;
};

TEST(SingleInstanceHosting, DisabledAllowsParallelHosts)
{
	std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.singleinstance.disabled", "Test", "Test", false) };
	host_options options{ info, std::span<char*>{} };
	options.set_single_instance(false);
	host h1{ options };
	host h2{ options };
	h1.get_services()->add<lifetime_service, gated_lifetime_service>(service_scope::singleton);
	h2.get_services()->add<lifetime_service, gated_lifetime_service>(service_scope::singleton);
	std::shared_ptr<gated_lifetime_service> life1{ std::static_pointer_cast<gated_lifetime_service>(h1.get_services()->get<lifetime_service>()) };
	std::shared_ptr<gated_lifetime_service> life2{ std::static_pointer_cast<gated_lifetime_service>(h2.get_services()->get<lifetime_service>()) };
	std::future<void> started1 = life1->get_started_future();
	std::future<void> started2 = life2->get_started_future();
	std::exception_ptr ptr1;
	std::exception_ptr ptr2;
	std::thread t1{ [&ptr1, &h1]()
	{
		ptr1 = h1.run();
	} };
	std::thread t2{ [&ptr2, &h2]()
	{
		ptr2 = h2.run();
	} };
	ASSERT_EQ(started1.wait_for(std::chrono::seconds(2)), std::future_status::ready);
	ASSERT_EQ(started2.wait_for(std::chrono::seconds(2)), std::future_status::ready);
	life1->release();
	life2->release();
	ASSERT_NO_THROW(t1.join());
	ASSERT_NO_THROW(t2.join());
	ASSERT_FALSE(ptr1);
	ASSERT_FALSE(ptr2);
}

TEST(SingleInstanceHosting, EnabledRejectsSecondHostWithExpectedError)
{
	std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.singleinstance.enabled", "Test", "Test", false) };
	host_options options{ info, std::span<char*>{} };
	options.set_single_instance(true);
	host h1{ options };
	host h2{ options };
	h1.get_services()->add<lifetime_service, gated_lifetime_service>(service_scope::singleton);
	h2.get_services()->add<lifetime_service, gated_lifetime_service>(service_scope::singleton);
	std::shared_ptr<gated_lifetime_service> life1{ std::static_pointer_cast<gated_lifetime_service>(h1.get_services()->get<lifetime_service>()) };
	std::future<void> started1 = life1->get_started_future();
	std::exception_ptr ptr1;
	std::exception_ptr ptr2;
	std::thread t1{ [&ptr1, &h1]()
	{
		ptr1 = h1.run();
	} };
	ASSERT_EQ(started1.wait_for(std::chrono::seconds(2)), std::future_status::ready);
	ptr2 = h2.run();
	life1->release();
	ASSERT_NO_THROW(t1.join());
	ASSERT_FALSE(ptr1);
	ASSERT_TRUE(ptr2);
	try
	{
		std::rethrow_exception(ptr2);
		FAIL();
	}
	catch (single_instance_running_error& e)
	{
		ASSERT_EQ(std::string{ e.what() }, "An instance of this single-instance application is already running");
	}
	catch (...)
	{
		FAIL();
	}
}
