#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;

static std::vector<char*> make_argv(std::vector<std::string>& args)
{
	std::vector<char*> argv{ args.size() };
	for (size_t i{ 0 }; i < args.size(); i++)
	{
		argv[i] = args[i].data();
	}
	return argv;
}

TEST(ArgumentsService, InitialStateFromArgv)
{
	std::vector<std::string> args{ "app", "--verbose", "--output", "file.txt" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_EQ(service.get_count(), 4);
}

TEST(ArgumentsService, EmptyArgv)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_EQ(service.get_count(), 0);
}

TEST(ArgumentsService, SingleArgv)
{
	std::vector<std::string> args{ "app" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_EQ(service.get_count(), 1);
}

TEST(ArgumentsService, GetAllReturnsAllArguments)
{
	std::vector<std::string> args{ "app", "--foo", "--bar" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	const auto all = service.get_all();
	ASSERT_EQ(all.size(), 3);
	ASSERT_EQ(all[0], "app");
	ASSERT_EQ(all[1], "--foo");
	ASSERT_EQ(all[2], "--bar");
}

TEST(ArgumentsService, GetAllOnEmptyIsEmpty)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_TRUE(service.get_all().empty());
}

TEST(ArgumentsService, ContainsExistingArgument)
{
	std::vector<std::string> args{ "app", "--verbose" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_TRUE(service.contains("--verbose"));
}

TEST(ArgumentsService, ContainsMissingArgument)
{
	std::vector<std::string> args{ "app", "--verbose" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_FALSE(service.contains("--output"));
}

TEST(ArgumentsService, ContainsIsCaseSensitive)
{
	std::vector<std::string> args{ "app", "--Verbose" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_FALSE(service.contains("--verbose"));
	ASSERT_TRUE(service.contains("--Verbose"));
}

TEST(ArgumentsService, ContainsOnEmptyArgv)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_FALSE(service.contains("--anything"));
}

TEST(ArgumentsService, GetNextReturnsValueAfterFlag)
{
	std::vector<std::string> args{ "app", "--output", "file.txt" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	const auto value = service.get_next("--output");
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(value.value(), "file.txt"); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(ArgumentsService, GetNextReturnsNulloptForMissingFlag)
{
	std::vector<std::string> args{ "app", "--verbose" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_FALSE(service.get_next("--output").has_value());
}

TEST(ArgumentsService, GetNextReturnsNulloptWhenFlagIsLast)
{
	std::vector<std::string> args{ "app", "--output" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_FALSE(service.get_next("--output").has_value());
}

TEST(ArgumentsService, GetNextOnEmptyArgv)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_FALSE(service.get_next("--output").has_value());
}

TEST(ArgumentsService, GetNextReturnsFirstOccurrence)
{
	std::vector<std::string> args{ "app", "--output", "first.txt", "--output", "second.txt" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	const auto value = service.get_next("--output");
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(value.value(), "first.txt"); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(ArgumentsService, AddIncreasesCount)
{
	std::vector<std::string> args{ "app" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--extra");
	ASSERT_EQ(service.get_count(), 2);
}

TEST(ArgumentsService, AddedArgumentIsContained)
{
	std::vector<std::string> args{ "app" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--extra");
	ASSERT_TRUE(service.contains("--extra"));
}

TEST(ArgumentsService, AddedArgumentAppearsInGetAll)
{
	std::vector<std::string> args{ "app" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--extra");
	const auto all = service.get_all();
	ASSERT_EQ(all.size(), 2);
	ASSERT_EQ(all[1], "--extra");
}

TEST(ArgumentsService, AddMultipleArguments)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--first");
	service.add("--second");
	service.add("--third");
	ASSERT_EQ(service.get_count(), 3);
	ASSERT_TRUE(service.contains("--first"));
	ASSERT_TRUE(service.contains("--second"));
	ASSERT_TRUE(service.contains("--third"));
}

TEST(ArgumentsService, AddToEmptyService)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--only");
	ASSERT_EQ(service.get_count(), 1);
	ASSERT_TRUE(service.contains("--only"));
}

TEST(ArgumentsService, AddedArgumentUsableWithGetNext)
{
	std::vector<std::string> args{};
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--output");
	service.add("result.txt");
	const auto value = service.get_next("--output");
	ASSERT_TRUE(value.has_value());
	ASSERT_EQ(value.value(), "result.txt"); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(ArgumentsService, ArgvIsNonNullForNonEmptyArgs)
{
	std::vector<std::string> args{ "app", "--verbose" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	ASSERT_NE(service.argv(), nullptr);
}

TEST(ArgumentsService, ArgvContentsMatchArguments)
{
	std::vector<std::string> args{ "app", "--verbose" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	char** raw = service.argv();
	ASSERT_EQ(std::string(raw[0]), "app");
	ASSERT_EQ(std::string(raw[1]), "--verbose");
}

TEST(ArgumentsService, ArgvReflectsAddedArgument)
{
	std::vector<std::string> args{ "app" };
	std::vector<char*> argv{ make_argv(args) };
	arguments_service service{ { argv.data(), args.size() } };
	service.add("--extra");
	char** raw = service.argv();
	ASSERT_EQ(std::string(raw[1]), "--extra");
}