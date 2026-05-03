#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::helpers;

TEST(HelpersTest, StringManip_Lower)
{
	EXPECT_EQ(string_manip::lower("Hello World"), "hello world");
}

TEST(HelpersTest, StringManip_Upper)
{
	EXPECT_EQ(string_manip::upper("Hello World"), "HELLO WORLD");
}

TEST(HelpersTest, StringManip_Trim_Whitespace)
{
	EXPECT_EQ(string_manip::trim("  hello  "), "hello");
}

TEST(HelpersTest, StringManip_Trim_Delimiter)
{
	EXPECT_EQ(string_manip::trim(",,hello,,", ','), "hello");
}

TEST(HelpersTest, StringManip_Join)
{
	EXPECT_EQ(string_manip::join({ "a", "b", "c" }, ", "), "a, b, c");
}

TEST(HelpersTest, StringManip_Join_Empty)
{
	EXPECT_EQ(string_manip::join({}, ", "), "");
}

TEST(HelpersTest, StringManip_Split_ByChar)
{
	std::vector<std::string> result{ string_manip::split("a,b,c", ',') };
	ASSERT_EQ(result.size(), 3u);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(HelpersTest, StringManip_Split_ByString)
{
	std::vector<std::string> result{ string_manip::split("a::b::c", "::") };
	ASSERT_EQ(result.size(), 3u);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(HelpersTest, StringManip_Split_ExcludeEmpty)
{
	std::vector<std::string> result{ string_manip::split("a,,b,,c", ',', false) };
	ASSERT_EQ(result.size(), 3u);
	EXPECT_EQ(result[0], "a");
	EXPECT_EQ(result[1], "b");
	EXPECT_EQ(result[2], "c");
}

TEST(HelpersTest, StringManip_ReplaceAll_String)
{
	EXPECT_EQ(string_manip::replace_all("hello world", "world", "there"), "hello there");
}

TEST(HelpersTest, StringManip_ReplaceAll_Char)
{
	EXPECT_EQ(string_manip::replace_all("hello", 'l', 'r'), "herro");
}

TEST(HelpersTest, StringManip_Quote)
{
	EXPECT_EQ(string_manip::quote("hello"), "\"hello\"");
}

TEST(HelpersTest, StringManip_Base64_Roundtrip)
{
	std::vector<std::byte> input{ std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'} };
	std::string encoded{ string_manip::base64_encode(input) };
	EXPECT_FALSE(encoded.empty());
	std::vector<std::byte> decoded{ string_manip::base64_decode(encoded) };
	EXPECT_EQ(decoded, input);
}

TEST(HelpersTest, StringManip_Str_Wstr_Roundtrip)
{
	std::string original{ "hello" };
	EXPECT_EQ(string_manip::str(string_manip::wstr(original)), original);
}

TEST(HelpersTest, Uuid_GetNew_NotEmpty)
{
	EXPECT_FALSE(uuid::get_new().empty());
}

TEST(HelpersTest, Uuid_GetNew_UniqueValues)
{
	EXPECT_NE(uuid::get_new(), uuid::get_new());
}
