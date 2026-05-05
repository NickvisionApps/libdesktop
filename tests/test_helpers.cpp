#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::helpers;

TEST(Helpers_Test, StringManip_base64DecodeEmptyInput)
{
	EXPECT_TRUE(string_manip::base64_decode("").empty());
}

TEST(Helpers_Test, StringManip_base64Roundtrip)
{
	std::vector<std::byte> input{ std::byte{ 'H' }, std::byte{ 'e' }, std::byte{ 'l' }, std::byte{ 'l' }, std::byte{ 'o' } };
	std::string encoded{ string_manip::base64_encode(input) };
	EXPECT_FALSE(encoded.empty());
	std::vector<std::byte> decoded{ string_manip::base64_decode(encoded) };
	EXPECT_EQ(decoded, input);
}

TEST(Helpers_Test, StringManip_filenameNormalize)
{
	EXPECT_FALSE(string_manip::filename_normalize("myfile.txt", false).empty());
	EXPECT_EQ(string_manip::filename_normalize("hello", false), "hello");
	EXPECT_TRUE(string_manip::filename_normalize("", false).empty());
}

TEST(Helpers_Test, StringManip_join)
{
	EXPECT_EQ(string_manip::join({ "a", "b", "c" }, ", "), "a, b, c");
}

TEST(Helpers_Test, StringManip_joinEmpty)
{
	EXPECT_EQ(string_manip::join({}, ", "), "");
}

TEST(Helpers_Test, StringManip_lower)
{
	EXPECT_EQ(string_manip::lower("Hello World"), "hello world");
}

TEST(Helpers_Test, StringManip_quote)
{
	EXPECT_EQ(string_manip::quote("hello"), "\"hello\"");
}

TEST(Helpers_Test, StringManip_replaceAllByChar)
{
	EXPECT_EQ(string_manip::replace_all("hello", 'l', 'r'), "herro");
}

TEST(Helpers_Test, StringManip_replaceAllByString)
{
	EXPECT_EQ(string_manip::replace_all("hello world", "world", "there"), "hello there");
}

TEST(Helpers_Test, StringManip_splitByChar)
{
	std::vector<std::string> result{ string_manip::split("a,b,c", ',') };
	ASSERT_EQ(result.size(), 3u);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(Helpers_Test, StringManip_splitByString)
{
	std::vector<std::string> result{ string_manip::split("a::b::c", "::") };
	ASSERT_EQ(result.size(), 3u);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(Helpers_Test, StringManip_splitExcludeEmpty)
{
	std::vector<std::string> result{ string_manip::split("a,,b,,c", ',', false) };
	ASSERT_EQ(result.size(), 3u);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(Helpers_Test, StringManip_strWstrRoundtrip)
{
	std::string original{ "hello" };
	EXPECT_EQ(string_manip::str(string_manip::wstr(original)), original);
}

TEST(Helpers_Test, StringManip_trimDelimiter)
{
	EXPECT_EQ(string_manip::trim(",,hello,,", ','), "hello");
}

TEST(Helpers_Test, StringManip_trimWhitespace)
{
	EXPECT_EQ(string_manip::trim("  hello  "), "hello");
}

TEST(Helpers_Test, StringManip_upper)
{
	EXPECT_EQ(string_manip::upper("Hello World"), "HELLO WORLD");
}

TEST(Helpers_Test, Uuid_getNewNotEmpty)
{
	EXPECT_FALSE(uuid::get_new().empty());
}

TEST(Helpers_Test, Uuid_getNewUniqueValues)
{
	EXPECT_NE(uuid::get_new(), uuid::get_new());
}
