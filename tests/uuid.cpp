#include <algorithm>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <ranges>
#include <unordered_set>

using namespace desktop::helpers;

TEST(Uuid, Length)
{
	ASSERT_EQ(uuid::get_new().length(), 36u);
}

TEST(Uuid, HasCorrectHyphens)
{
	std::string id{ uuid::get_new() };
	ASSERT_EQ(id[8], '-');
	ASSERT_EQ(id[13], '-');
	ASSERT_EQ(id[18], '-');
	ASSERT_EQ(id[23], '-');
}

TEST(Uuid, ContainsValidCharacters)
{
	std::string id{ uuid::get_new() };
	ASSERT_TRUE(std::ranges::all_of(id, [](char c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '-';
	}));
}

TEST(Uuid, SegmentLengths)
{
	std::string id{ uuid::get_new() };
	ASSERT_EQ(id.substr(0, 8).length(), 8u);
	ASSERT_EQ(id.substr(9, 4).length(), 4u);
	ASSERT_EQ(id.substr(14, 4).length(), 4u);
	ASSERT_EQ(id.substr(19, 4).length(), 4u);
	ASSERT_EQ(id.substr(24, 12).length(), 12u);
}

TEST(Uuid, DifferentValues)
{
	ASSERT_NE(uuid::get_new(), uuid::get_new());
}

TEST(Uuid, LargeSet)
{
	std::unordered_set<std::string> ids;
	for (int i{ 0 }; i < 10000; ++i)
	{
		ids.insert(uuid::get_new());
	}
	ASSERT_EQ(static_cast<int>(ids.size()), 10000);
}