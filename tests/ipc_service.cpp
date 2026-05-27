#include <gtest/gtest.h>
#include <libdesktop.h>
#include <memory>

using namespace desktop::app;
using namespace desktop::helpers;
using namespace desktop::events;

TEST(IPCService, HelloWorld)
{
	std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.ipc", "Test", "Test", false) };
	bool received{ false };
	ipc_service ipc1{ info };
	ipc1.get_message_received_event() += [&received](const ipc_service&, const param_event_args<std::string>& message)
	{
		received = *message == "Hello World!";
	};
	ASSERT_TRUE(ipc1.is_host());
	ipc_service ipc2{ info };
	ASSERT_FALSE(ipc2.is_host());
	ASSERT_TRUE(ipc2.send_message("Hello World!"));
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	ASSERT_TRUE(received);
}

TEST(IPCService, Base64)
{
	std::string test{ "Testing this wonderful library<>:?" };
	std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.ipc", "Test", "Test", false) };
	bool received{ false };
	ipc_service ipc1{ info };
	ipc1.get_message_received_event() += [&test, &received](const ipc_service&, const param_event_args<std::string>& message)
	{
		received = string_manip::base64_decode(*message) ==
		           std::vector<std::byte>{ reinterpret_cast<const std::byte*>(test.data()), reinterpret_cast<const std::byte*>(test.data()) + test.size() };
	};
	ASSERT_TRUE(ipc1.is_host());
	ipc_service ipc2{ info };
	ASSERT_FALSE(ipc2.is_host());
	ASSERT_TRUE(ipc2.send_message(
	    string_manip::base64_encode({ reinterpret_cast<const std::byte*>(test.data()), reinterpret_cast<const std::byte*>(test.data()) + test.size() })));
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	ASSERT_TRUE(received);
}