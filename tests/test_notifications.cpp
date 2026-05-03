#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::events;
using namespace desktop::notifications;

TEST(Notifications_Test, Notification_copyConstruct)
{
	notification original{ "Hello", notification_severity::information };
	notification copy{ original };
	EXPECT_EQ(copy.get_message(), original.get_message());
	EXPECT_EQ(copy.get_severity(), original.get_severity());
}

TEST(Notifications_Test, Notification_defaultActionEmpty)
{
	notification n{ "msg", notification_severity::information };
	EXPECT_TRUE(n.get_action().empty());
	EXPECT_TRUE(n.get_action_parameter().empty());
}

TEST(Notifications_Test, Notification_moveConstruct)
{
	notification original{ "Hello", notification_severity::information };
	notification moved{ std::move(original) };
	EXPECT_EQ(moved.get_message(), "Hello");
}

TEST(Notifications_Test, Notification_setAction)
{
	notification n{ "msg", notification_severity::information };
	n.set_action("open", "/path/to/file");
	EXPECT_EQ(n.get_action(), "open");
	EXPECT_EQ(n.get_action_parameter(), "/path/to/file");
}

TEST(Notifications_Test, Notification_storesMessage)
{
	notification n{ "Hello", notification_severity::information };
	EXPECT_EQ(n.get_message(), "Hello");
}

TEST(Notifications_Test, Notification_storesSeverity)
{
	notification n{ "msg", notification_severity::error };
	EXPECT_EQ(n.get_severity(), notification_severity::error);
}

TEST(Notifications_Test, NotificationSentEventArgs_containsSentNotification)
{
	notification_service svc;
	std::shared_ptr<notification> sent{ std::make_shared<notification>("Hello", notification_severity::success) };
	std::shared_ptr<notification> received;
	svc.get_notification_sent_event().add_handler([&received](const notification_service&, const notification_sent_event_args& args)
	{
		received = args.get_notification();
	});
	svc.send(sent);
	ASSERT_NE(received, nullptr);
	EXPECT_EQ(received->get_message(), "Hello");
	EXPECT_EQ(received->get_severity(), notification_severity::success);
}

TEST(Notifications_Test, NotificationSentEventArgs_getTimestamp)
{
	notification_service svc;
	std::chrono::system_clock::time_point before{ std::chrono::system_clock::now() };
	std::chrono::system_clock::time_point timestamp;
	svc.get_notification_sent_event().add_handler([&timestamp](const notification_service&, const notification_sent_event_args& args)
	{
		timestamp = args.get_timestamp();
	});
	svc.send(std::make_shared<notification>("Test", notification_severity::information));
	std::chrono::system_clock::time_point after{ std::chrono::system_clock::now() };
	EXPECT_GE(timestamp, before);
	EXPECT_LE(timestamp, after);
}

TEST(Notifications_Test, NotificationService_handlersInvokedOnSend)
{
	notification_service svc;
	int count{ 0 };
	event_id id1{ svc.get_notification_sent_event().add_handler([&count](const notification_service&, const notification_sent_event_args&) { count++; }) };
	event_id id2{ svc.get_notification_sent_event().add_handler([&count](const notification_service&, const notification_sent_event_args&) { count++; }) };
	svc.send(std::make_shared<notification>("Test", notification_severity::information));
	EXPECT_EQ(count, 2);
	svc.get_notification_sent_event().remove_handler(id1);
	svc.get_notification_sent_event().remove_handler(id2);
}

TEST(Notifications_Test, NotificationService_removedHandlerNotInvoked)
{
	notification_service svc;
	int count{ 0 };
	event_id id{ svc.get_notification_sent_event().add_handler([&count](const notification_service&, const notification_sent_event_args&) { count++; }) };
	svc.get_notification_sent_event().remove_handler(id);
	svc.send(std::make_shared<notification>("Test", notification_severity::information));
	EXPECT_EQ(count, 0);
}

TEST(Notifications_Test, ShellNotification_getTitle)
{
	shell_notification n{ "My Title", "My Message", notification_severity::information };
	EXPECT_EQ(n.get_title(), "My Title");
	EXPECT_EQ(n.get_message(), "My Message");
	EXPECT_EQ(n.get_severity(), notification_severity::information);
}
