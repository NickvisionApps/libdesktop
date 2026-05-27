#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::helpers;

TEST(StringManip, Base64RoundTrip)
{
	std::string test{ "Hello, World!" };
	std::vector<std::byte> bytes{ reinterpret_cast<const std::byte*>(test.data()), reinterpret_cast<const std::byte*>(test.data()) + test.size() };
	ASSERT_EQ(string_manip::base64_decode(string_manip::base64_encode(bytes)), bytes);
}

TEST(StringManip, Base64EncodeKnownValue)
{
	std::string test{ "Man" };
	std::vector<std::byte> bytes{ reinterpret_cast<const std::byte*>(test.data()), reinterpret_cast<const std::byte*>(test.data()) + test.size() };
	ASSERT_EQ(string_manip::base64_encode(bytes), "TWFu");
}

TEST(StringManip, Base64DecodeKnownValue)
{
	std::string expected{ "Man" };
	std::vector<std::byte> expected_bytes{ reinterpret_cast<const std::byte*>(expected.data()),
		                                   reinterpret_cast<const std::byte*>(expected.data()) + expected.size() };
	ASSERT_EQ(string_manip::base64_decode("TWFu"), expected_bytes);
}

TEST(StringManip, Base64EmptyInput)
{
	std::vector<std::byte> empty{};
	ASSERT_EQ(string_manip::base64_encode(empty), "");
	ASSERT_EQ(string_manip::base64_decode(""), empty);
}

TEST(StringManip, Base64SpecialCharacters)
{
	std::string test{ "Testing this wonderful library<>:?" };
	std::vector<std::byte> bytes{ reinterpret_cast<const std::byte*>(test.data()), reinterpret_cast<const std::byte*>(test.data()) + test.size() };
	ASSERT_EQ(string_manip::base64_decode(string_manip::base64_encode(bytes)), bytes);
}

TEST(StringManip, FilenameNormalizeWindowsReplacesIllegalChars)
{
	std::string result{ string_manip::filename_normalize("file<name>:test", true) };
	ASSERT_EQ(result.find('<'), std::string::npos);
	ASSERT_EQ(result.find('>'), std::string::npos);
	ASSERT_EQ(result.find(':'), std::string::npos);
}

TEST(StringManip, FilenameNormalizeNonWindowsPreservesColons)
{
	std::string filename{ "file:name" };
	std::string result{ string_manip::filename_normalize(filename, false) };
	ASSERT_NE(result.find(':'), std::string::npos);
}

TEST(StringManip, FilenameNormalizeEmptyString)
{
	ASSERT_NO_THROW(string_manip::filename_normalize("", true));
	ASSERT_NO_THROW(string_manip::filename_normalize("", false));
}

TEST(StringManip, FilenameNormalizeCleanName)
{
	std::string filename{ "my_file_name" };
	ASSERT_EQ(string_manip::filename_normalize(filename, true), filename);
	ASSERT_EQ(string_manip::filename_normalize(filename, false), filename);
}

TEST(StringManip, JoinBasic)
{
	std::vector<std::string> parts{ "a", "b", "c" };
	ASSERT_EQ(string_manip::join(parts, ", "), "a, b, c");
}

TEST(StringManip, JoinSingleElement)
{
	std::vector<std::string> parts{ "only" };
	ASSERT_EQ(string_manip::join(parts, ", "), "only");
}

TEST(StringManip, JoinEmptyVector)
{
	std::vector<std::string> parts{};
	ASSERT_EQ(string_manip::join(parts, ", "), "");
}

TEST(StringManip, JoinEmptyDelimiter)
{
	std::vector<std::string> parts{ "a", "b", "c" };
	ASSERT_EQ(string_manip::join(parts, ""), "abc");
}

TEST(StringManip, JoinMultiCharDelimiter)
{
	std::vector<std::string> parts{ "foo", "bar", "baz" };
	ASSERT_EQ(string_manip::join(parts, " | "), "foo | bar | baz");
}

TEST(StringManip, LowerBasic)
{
	ASSERT_EQ(string_manip::lower("Hello World"), "hello world");
}

TEST(StringManip, LowerAlreadyLower)
{
	ASSERT_EQ(string_manip::lower("hello"), "hello");
}

TEST(StringManip, LowerEmpty)
{
	ASSERT_EQ(string_manip::lower(""), "");
}

TEST(StringManip, UpperBasic)
{
	ASSERT_EQ(string_manip::upper("Hello World"), "HELLO WORLD");
}

TEST(StringManip, UpperAlreadyUpper)
{
	ASSERT_EQ(string_manip::upper("HELLO"), "HELLO");
}

TEST(StringManip, UpperEmpty)
{
	ASSERT_EQ(string_manip::upper(""), "");
}

TEST(StringManip, LowerUpperRoundTrip)
{
	std::string original{ "MiXeDcAsE" };
	ASSERT_EQ(string_manip::upper(string_manip::lower(original)), "MIXEDCASE");
}

TEST(StringManip, QuoteBasic)
{
	ASSERT_EQ(string_manip::quote("hello"), "\"hello\"");
}

TEST(StringManip, QuoteEmpty)
{
	ASSERT_EQ(string_manip::quote(""), "\"\"");
}

TEST(StringManip, QuoteWithSpaces)
{
	ASSERT_EQ(string_manip::quote("hello world"), "\"hello world\"");
}

TEST(StringManip, ReplaceAllStringBasic)
{
	ASSERT_EQ(string_manip::replace_all("foo bar foo", "foo", "baz"), "baz bar baz");
}

TEST(StringManip, ReplaceAllStringNoMatch)
{
	ASSERT_EQ(string_manip::replace_all("hello world", "xyz", "abc"), "hello world");
}

TEST(StringManip, ReplaceAllStringEmpty)
{
	ASSERT_EQ(string_manip::replace_all("", "foo", "bar"), "");
}

TEST(StringManip, ReplaceAllStringEmptyFrom)
{
	ASSERT_NO_THROW(string_manip::replace_all("hello", "", "x"));
}

TEST(StringManip, ReplaceAllCharBasic)
{
	ASSERT_EQ(string_manip::replace_all("a.b.c", '.', '/'), "a/b/c");
}

TEST(StringManip, ReplaceAllCharNoMatch)
{
	ASSERT_EQ(string_manip::replace_all("hello", 'z', 'x'), "hello");
}

TEST(StringManip, ReplaceAllCharEmpty)
{
	ASSERT_EQ(string_manip::replace_all("", 'a', 'b'), "");
}

TEST(StringManip, WstrAndStrRoundTrip)
{
	std::string original{ "Hello World" };
	ASSERT_EQ(string_manip::str(string_manip::wstr(original)), original);
}

TEST(StringManip, WstrEmpty)
{
	ASSERT_EQ(string_manip::wstr(""), L"");
}

TEST(StringManip, StrEmpty)
{
	ASSERT_EQ(string_manip::str(L""), "");
}

TEST(StringManip, WstrKnownValue)
{
	ASSERT_EQ(string_manip::wstr("abc"), L"abc");
}

TEST(StringManip, StrKnownValue)
{
	ASSERT_EQ(string_manip::str(L"abc"), "abc");
}

TEST(StringManip, TrimBothSides)
{
	ASSERT_EQ(string_manip::trim("  hello  "), "hello");
}

TEST(StringManip, TrimLeadingOnly)
{
	ASSERT_EQ(string_manip::trim("  hello"), "hello");
}

TEST(StringManip, TrimTrailingOnly)
{
	ASSERT_EQ(string_manip::trim("hello  "), "hello");
}

TEST(StringManip, TrimNoWhitespace)
{
	ASSERT_EQ(string_manip::trim("hello"), "hello");
}

TEST(StringManip, TrimEmpty)
{
	ASSERT_EQ(string_manip::trim(""), "");
}

TEST(StringManip, TrimOnlyWhitespace)
{
	ASSERT_EQ(string_manip::trim("    "), "");
}

TEST(StringManip, TrimDelimiterBasic)
{
	ASSERT_EQ(string_manip::trim(",,hello,,", ','), "hello");
}

TEST(StringManip, TrimDelimiterNoMatch)
{
	ASSERT_EQ(string_manip::trim("hello", ','), "hello");
}

TEST(StringManip, TrimDelimiterEmpty)
{
	ASSERT_EQ(string_manip::trim("", ','), "");
}

TEST(StringManip, SplitBasic)
{
	std::vector<std::string> expected{ "a", "b", "c" };
	ASSERT_EQ(string_manip::split("a,b,c", ","), expected);
}

TEST(StringManip, SplitMultiCharDelimiter)
{
	std::vector<std::string> expected{ "foo", "bar", "baz" };
	ASSERT_EQ(string_manip::split("foo::bar::baz", "::"), expected);
}

TEST(StringManip, SplitIncludeEmpty)
{
	std::vector<std::string> expected{ "a", "", "c" };
	ASSERT_EQ(string_manip::split("a,,c", ",", true), expected);
}

TEST(StringManip, SplitExcludeEmpty)
{
	std::vector<std::string> expected{ "a", "c" };
	ASSERT_EQ(string_manip::split("a,,c", ",", false), expected);
}

TEST(StringManip, SplitNoDelimiterFound)
{
	std::vector<std::string> expected{ "hello" };
	ASSERT_EQ(string_manip::split("hello", ","), expected);
}

TEST(StringManip, SplitEmptyString)
{
	std::vector<std::string> result{ string_manip::split("", ",") };
	ASSERT_EQ(result.size(), 1U);
	ASSERT_EQ(result[0], "");
}

TEST(StringManip, SplitEmptyStringExcludeEmpty)
{
	std::vector<std::string> result{ string_manip::split("", ",", false) };
	ASSERT_TRUE(result.empty());
}

TEST(StringManip, SplitCharBasic)
{
	std::vector<std::string> expected{ "a", "b", "c" };
	ASSERT_EQ(string_manip::split("a,b,c", ','), expected);
}

TEST(StringManip, SplitCharIncludeEmpty)
{
	std::vector<std::string> expected{ "a", "", "c" };
	ASSERT_EQ(string_manip::split("a,,c", ',', true), expected);
}

TEST(StringManip, SplitCharExcludeEmpty)
{
	std::vector<std::string> expected{ "a", "c" };
	ASSERT_EQ(string_manip::split("a,,c", ',', false), expected);
}

TEST(StringManip, SplitCharNoMatch)
{
	std::vector<std::string> expected{ "hello" };
	ASSERT_EQ(string_manip::split("hello", ','), expected);
}

TEST(StringManip, SplitCharEmptyString)
{
	std::vector<std::string> result{ string_manip::split("", ',') };
	ASSERT_EQ(result.size(), 1U);
	ASSERT_EQ(result[0], "");
}