#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::events;
using namespace desktop::notifications;

TEST(NotificationsTest, Notification_StoresMessage)
{
	notification n{ "Hello", notification_severity::information };
	EXPECT_EQ(n.get_message(), "Hello");
}

TEST(NotificationsTest, Notification_StoresSeverity)
{
	notification n{ "msg", notification_severity::error };
	EXPECT_EQ(n.get_severity(), notification_severity::error);
}

TEST(NotificationsTest, Notification_DefaultActionEmpty)
{
	notification n{ "msg", notification_severity::information };
	EXPECT_TRUE(n.get_action().empty());
	EXPECT_TRUE(n.get_action_parameter().empty());
}

TEST(NotificationsTest, Notification_SetAction)
{
	notification n{ "msg", notification_severity::information };
	n.set_action("open", "/path/to/file");
	EXPECT_EQ(n.get_action(), "open");
	EXPECT_EQ(n.get_action_parameter(), "/path/to/file");
}

TEST(NotificationsTest, NotificationService_HandlersInvokedOnSend)
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

TEST(NotificationsTest, NotificationService_RemovedHandlerNotInvoked)
{
	notification_service svc;
	int count{ 0 };
	event_id id{ svc.get_notification_sent_event().add_handler([&count](const notification_service&, const notification_sent_event_args&) { count++; }) };
	svc.get_notification_sent_event().remove_handler(id);
	svc.send(std::make_shared<notification>("Test", notification_severity::information));
	EXPECT_EQ(count, 0);
}

TEST(NotificationsTest, NotificationSentEventArgs_ContainsSentNotification)
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
