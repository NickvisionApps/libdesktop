#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::events;

struct test_sender {};

TEST(Events_Test, Event_addHandler)
{
	event<test_sender, event_args> e;
	test_sender sender;
	event_id id1{ e.add_handler([](const test_sender&, const event_args&) {}) };
	event_id id2{ e.add_handler([](const test_sender&, const event_args&) {}) };
	EXPECT_NE(id1, id2);
}

TEST(Events_Test, Event_invokeCallsAllHandlers)
{
	event<test_sender, event_args> e;
	test_sender sender;
	int count{ 0 };
	e.add_handler([&count](const test_sender&, const event_args&) { count++; });
	e.add_handler([&count](const test_sender&, const event_args&) { count++; });
	e.invoke(sender, event_args{});
	EXPECT_EQ(count, 2);
}

TEST(Events_Test, Event_invokePassesSender)
{
	event<test_sender, event_args> e;
	test_sender sender;
	const test_sender* received{ nullptr };
	e.add_handler([&received](const test_sender& s, const event_args&) { received = &s; });
	e.invoke(sender, event_args{});
	EXPECT_EQ(received, &sender);
}

TEST(Events_Test, Event_removeHandler)
{
	event<test_sender, event_args> e;
	test_sender sender;
	int count{ 0 };
	event_id id{ e.add_handler([&count](const test_sender&, const event_args&) { count++; }) };
	e.remove_handler(id);
	e.invoke(sender, event_args{});
	EXPECT_EQ(count, 0);
}

TEST(Events_Test, Event_removeNonExistentHandler)
{
	event<test_sender, event_args> e;
	test_sender sender;
	EXPECT_NO_THROW(e.remove_handler(9999));
	int count{ 0 };
	e.add_handler([&count](const test_sender&, const event_args&) { count++; });
	e.invoke(sender, event_args{});
	EXPECT_EQ(count, 1);
}

TEST(Events_Test, ParamEventArgs_value)
{
	param_event_args<int> args{ 42 };
	EXPECT_EQ(args.value(), 42);
}
