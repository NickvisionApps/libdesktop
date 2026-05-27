#include <gtest/gtest.h>
#include <libdesktop.h>
#include <string>
#include <thread>
#include <vector>

using namespace desktop::events;

struct sender_t
{
};

TEST(Events, EventArgsDefaultConstructible)
{
	ASSERT_NO_THROW(event_args{});
}

TEST(Events, EventArgsCopyConstructible)
{
	event_args a{};
	ASSERT_NO_THROW(event_args b{ a });
}

TEST(Events, EventArgsMoveConstructible)
{
	event_args a{};
	ASSERT_NO_THROW(event_args b{ std::move(a) });
}

TEST(Events, ParamGetValue)
{
	param_event_args<int> args{ 42 };
	ASSERT_EQ(args.get_value(), 42);
}

TEST(Events, ParamDereference)
{
	param_event_args<int> args{ 7 };
	ASSERT_EQ(*args, 7);
}

TEST(Events, ParamCopy)
{
	param_event_args<int> a{ 10 };
	param_event_args<int> b{ a };
	ASSERT_EQ(*b, 10);
}

TEST(Events, ParamMove)
{
	param_event_args<int> a{ 10 };
	param_event_args<int> b{ std::move(a) };
	ASSERT_EQ(*b, 10);
}

TEST(Events, ParamIsEventArgs)
{
	param_event_args<int> args{ 1 };
	ASSERT_NE(dynamic_cast<event_args*>(&args), nullptr);
}

TEST(Events, ParamClassGetValue)
{
	param_event_args<std::string> args{ "hello" };
	ASSERT_EQ(args.get_value(), "hello");
}

TEST(Events, ParamClassDereferenceOperator)
{
	param_event_args<std::string> args{ "hello" };
	ASSERT_EQ(*args, "hello");
}

TEST(Events, ParamClassArrowOperator)
{
	param_event_args<std::string> args{ "hello" };
	ASSERT_EQ(args->length(), 5u);
}

TEST(Events, ParamClassGetValueReturnsConstRef)
{
	param_event_args<std::string> args{ "hello" };
	const std::string& ref{ args.get_value() };
	ASSERT_EQ(ref, "hello");
}

TEST(Events, ParamClassCopyConstructor)
{
	param_event_args<std::string> a{ "hello" };
	param_event_args<std::string> b{ a };
	ASSERT_EQ(*b, "hello");
}

TEST(Events, ParamClassMoveConstructor)
{
	param_event_args<std::string> a{ "hello" };
	param_event_args<std::string> b{ std::move(a) };
	ASSERT_EQ(*b, "hello");
}

TEST(Events, ParamClassIsEventArgs)
{
	param_event_args<std::string> args{ "hello" };
	ASSERT_NE(dynamic_cast<event_args*>(&args), nullptr);
}

TEST(Events, HandlerIsInvoked)
{
	event<sender_t, event_args> e;
	bool called{ false };
	e += [&called](const sender_t&, const event_args&)
	{
		called = true;
	};
	e.invoke({}, {});
	ASSERT_TRUE(called);
}

TEST(Events, HandlerReceivesSender)
{
	event<sender_t, param_event_args<int>> e;
	const sender_t sender{};
	const sender_t* received{ nullptr };
	e += [&received, &sender](const sender_t& s, const param_event_args<int>&)
	{
		received = &s;
	};
	e.invoke(sender, param_event_args<int>{ 0 });
	ASSERT_EQ(received, &sender);
}

TEST(Events, HandlerReceivesArgs)
{
	event<sender_t, param_event_args<int>> e;
	int received{ 0 };
	e += [&received](const sender_t&, const param_event_args<int>& args)
	{
		received = *args;
	};
	e.invoke({}, param_event_args<int>{ 99 });
	ASSERT_EQ(received, 99);
}

TEST(Events, MultipleHandlersAllInvoked)
{
	event<sender_t, event_args> e;
	int count{ 0 };
	e += [&count](const sender_t&, const event_args&)
	{
		++count;
	};
	e += [&count](const sender_t&, const event_args&)
	{
		++count;
	};
	e += [&count](const sender_t&, const event_args&)
	{
		++count;
	};
	e.invoke({}, {});
	ASSERT_EQ(count, 3);
}

TEST(Events, AddHandlerReturnsUniqueIds)
{
	event<sender_t, event_args> e;
	event_id id1{ e.add_handler([](const sender_t&, const event_args&)
	{
	}) };
	event_id id2{ e.add_handler([](const sender_t&, const event_args&)
	{
	}) };
	event_id id3{ e.add_handler([](const sender_t&, const event_args&)
	{
	}) };
	ASSERT_NE(id1, id2);
	ASSERT_NE(id2, id3);
	ASSERT_NE(id1, id3);
}

TEST(Events, RemovedHandlerIsNotInvoked)
{
	event<sender_t, event_args> e;
	bool called{ false };
	event_id id{ e.add_handler([&called](const sender_t&, const event_args&)
	{
		called = true;
	}) };
	e.remove_handler(id);
	e.invoke({}, {});
	ASSERT_FALSE(called);
}

TEST(Events, OperatorMinusEqualsRemovesHandler)
{
	event<sender_t, event_args> e;
	bool called{ false };
	event_id id{ e.add_handler([&called](const sender_t&, const event_args&)
	{
		called = true;
	}) };
	e -= id;
	e.invoke({}, {});
	ASSERT_FALSE(called);
}

TEST(Events, RemovingOneHandlerLeavesOthersIntact)
{
	event<sender_t, event_args> e;
	int count{ 0 };
	event_id id{ e.add_handler([](const sender_t&, const event_args&)
	{
	}) };
	e += [&count](const sender_t&, const event_args&)
	{
		++count;
	};
	e += [&count](const sender_t&, const event_args&)
	{
		++count;
	};
	e.remove_handler(id);
	e.invoke({}, {});
	ASSERT_EQ(count, 2);
}

TEST(Events, RemovingNonExistentIdDoesNotThrow)
{
	event<sender_t, event_args> e;
	ASSERT_NO_THROW(e.remove_handler(9999));
}

TEST(Events, InvokeWithNoHandlersDoesNotThrow)
{
	event<sender_t, event_args> e;
	ASSERT_NO_THROW(e.invoke({}, {}));
}

TEST(Events, StringArgPassedCorrectly)
{
	event<sender_t, param_event_args<std::string>> e;
	std::string received{};
	e += [&received](const sender_t&, const param_event_args<std::string>& args)
	{
		received = *args;
	};
	e.invoke({}, param_event_args<std::string>{ "hello world" });
	ASSERT_EQ(received, "hello world");
}

TEST(Events, ConcurrentInvokeDoesNotCrash)
{
	event<sender_t, event_args> e;
	std::atomic<int> count{ 0 };
	e += [&count](const sender_t&, const event_args&)
	{
		++count;
	};
	std::vector<std::thread> threads;
	for (int i{ 0 }; i < 8; ++i)
	{
		threads.emplace_back([&e]()
		{
			for (int j{ 0 }; j < 100; ++j)
			{
				e.invoke({}, {});
			}
		});
	}
	for (std::thread& t : threads)
	{
		t.join();
	}
	ASSERT_EQ(count.load(), 800);
}

TEST(Events, ConcurrentAddAndInvokeDoesNotCrash)
{
	event<sender_t, event_args> e;
	std::atomic<bool> done{ false };
	std::thread invoker{ [&e, &done]()
	{
		while (!done.load())
		{
			e.invoke({}, {});
		}
	} };
	for (int i{ 0 }; i < 100; ++i)
	{
		e.add_handler([](const sender_t&, const event_args&)
		{
		});
	}
	done.store(true);
	invoker.join();
}

TEST(Events, ConcurrentAddAndRemoveDoesNotCrash)
{
	event<sender_t, event_args> e;
	std::vector<event_id> ids;
	std::mutex ids_mutex;
	std::thread adder{ [&]()
	{
		for (int i{ 0 }; i < 100; ++i)
		{
			event_id id{ e.add_handler([](const sender_t&, const event_args&)
			{
			}) };
			std::scoped_lock lock{ ids_mutex };
			ids.push_back(id);
		}
	} };
	std::thread remover{ [&]()
	{
		for (int i{ 0 }; i < 50; ++i)
		{
			std::scoped_lock lock{ ids_mutex };
			if (!ids.empty())
			{
				e.remove_handler(ids.back());
				ids.pop_back();
			}
		}
	} };
	adder.join();
	remover.join();
}