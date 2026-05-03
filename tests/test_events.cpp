#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::events;

struct test_sender {};

TEST(EventsTest, AddHandler_ReturnsUniqueIds)
{
	event<test_sender, event_args> e;
	test_sender sender;
	event_id id1{ e.add_handler([](const test_sender&, const event_args&) {}) };
	event_id id2{ e.add_handler([](const test_sender&, const event_args&) {}) };
	EXPECT_NE(id1, id2);
}

TEST(EventsTest, Invoke_CallsAllRegisteredHandlers)
{
	event<test_sender, event_args> e;
	test_sender sender;
	int count{ 0 };
	e.add_handler([&count](const test_sender&, const event_args&) { count++; });
	e.add_handler([&count](const test_sender&, const event_args&) { count++; });
	e.invoke(sender, event_args{});
	EXPECT_EQ(count, 2);
}

TEST(EventsTest, RemoveHandler_NotInvokedAfterRemoval)
{
	event<test_sender, event_args> e;
	test_sender sender;
	int count{ 0 };
	event_id id{ e.add_handler([&count](const test_sender&, const event_args&) { count++; }) };
	e.remove_handler(id);
	e.invoke(sender, event_args{});
	EXPECT_EQ(count, 0);
}

TEST(EventsTest, Invoke_PassesSenderToHandler)
{
	event<test_sender, event_args> e;
	test_sender sender;
	const test_sender* received{ nullptr };
	e.add_handler([&received](const test_sender& s, const event_args&) { received = &s; });
	e.invoke(sender, event_args{});
	EXPECT_EQ(received, &sender);
}
