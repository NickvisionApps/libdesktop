#include <chrono>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::notifications;

class NotificationService : public testing::Test
{
protected:
	void SetUp() override
	{
		m_info = std::make_shared<app_info>("libdesktop.test.notification", "NotificationServiceTests", "NotificationServiceTests", false);
		m_translation_service = std::make_shared<translation_service>(m_info);
		m_service = std::make_shared<notification_service>(m_info, m_translation_service);
	}

	void TearDown() override
	{
		m_service.reset();
		m_translation_service.reset();
		m_info.reset();
	}

	std::shared_ptr<app_info> m_info;
	std::shared_ptr<translation_service> m_translation_service;
	std::shared_ptr<notification_service> m_service;
};

TEST_F(NotificationService, AppNotificationEventFiresOnSend)
{
	bool fired{ false };
	m_service->get_app_notification_sent_event().add_handler([&](const notification_service&, const app_notification_sent_event_args&)
	{
		fired = true;
	});
	m_service->send(app_notification{ "message", notification_severity::information });
	ASSERT_TRUE(fired);
}

TEST_F(NotificationService, AppNotificationEventContainsNotificationData)
{
	std::string received_message;
	notification_severity received_severity{ notification_severity::information };
	std::string received_action;
	std::string received_action_parameter;
	m_service->get_app_notification_sent_event().add_handler([&](const notification_service&, const app_notification_sent_event_args& args)
	{
		received_message = args.get_notification().get_message();
		received_severity = args.get_notification().get_severity();
		received_action = args.get_notification().get_action();
		received_action_parameter = args.get_notification().get_action_parameter();
	});
	app_notification notification{ "sync complete", notification_severity::success };
	notification.set_action("open", "/tmp/file.txt");
	m_service->send(notification);
	ASSERT_EQ(received_message, "sync complete");
	ASSERT_EQ(received_severity, notification_severity::success);
	ASSERT_EQ(received_action, "open");
	ASSERT_EQ(received_action_parameter, "/tmp/file.txt");
}

TEST_F(NotificationService, AppNotificationEventTimestampIsCurrent)
{
	std::chrono::system_clock::time_point before_send = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point received_timestamp{};
	m_service->get_app_notification_sent_event().add_handler([&](const notification_service&, const app_notification_sent_event_args& args)
	{
		received_timestamp = args.get_timestamp();
	});
	m_service->send(app_notification{ "message", notification_severity::warning });
	std::chrono::system_clock::time_point after_send = std::chrono::system_clock::now();
	ASSERT_GE(received_timestamp, before_send);
	ASSERT_LE(received_timestamp, after_send);
}

TEST_F(NotificationService, AppNotificationEventSenderMatchesService)
{
	bool sender_matches{ false };
	m_service->get_app_notification_sent_event().add_handler([&](const notification_service& service, const app_notification_sent_event_args&)
	{
		sender_matches = &service == m_service.get();
	});
	m_service->send(app_notification{ "message", notification_severity::error });
	ASSERT_TRUE(sender_matches);
}

TEST_F(NotificationService, SendShellNotificationDoesNotThrow)
{
	shell_notification notification{ "title", "message", notification_severity::information };
	ASSERT_NO_THROW(m_service->send(notification));
}
