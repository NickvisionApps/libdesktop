#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::updates;

TEST(Version, DefaultIsEmpty)
{
	version v;
	ASSERT_EQ(v.get_major(), 0);
	ASSERT_EQ(v.get_minor(), 0);
	ASSERT_EQ(v.get_patch(), 0);
	ASSERT_TRUE(v.get_preview().empty());
	ASSERT_TRUE(v.empty());
	ASSERT_FALSE(static_cast<bool>(v));
	ASSERT_EQ(v.str(), "0.0.0");
}

TEST(Version, ThreePartConstructor)
{
	version v{ 1, 2, 3 };
	ASSERT_EQ(v.get_major(), 1);
	ASSERT_EQ(v.get_minor(), 2);
	ASSERT_EQ(v.get_patch(), 3);
	ASSERT_FALSE(v.is_preview());
	ASSERT_EQ(v.str(), "1.2.3");
}

TEST(Version, PreviewIntConstructor)
{
	version v{ 1, 2, 3, 4 };
	ASSERT_TRUE(v.is_preview());
	ASSERT_EQ(v.get_preview(), "4");
	ASSERT_EQ(v.str(), "1.2.3-4");
}

TEST(Version, PreviewStringConstructorThrowsOnEmptyPreview)
{
	ASSERT_THROW(version(1, 2, 3, ""), std::invalid_argument);
}

TEST(Version, StringConstructorParsesPrefixedVersion)
{
	version v{ "v2.10.7-beta" };
	ASSERT_EQ(v.get_major(), 2);
	ASSERT_EQ(v.get_minor(), 10);
	ASSERT_EQ(v.get_patch(), 7);
	ASSERT_EQ(v.get_preview(), "beta");
	ASSERT_EQ(v.str(), "2.10.7-beta");
}

TEST(Version, StringConstructorParsesDotPreview)
{
	version v{ "3.4.5.rc1" };
	ASSERT_EQ(v.get_major(), 3);
	ASSERT_EQ(v.get_minor(), 4);
	ASSERT_EQ(v.get_patch(), 5);
	ASSERT_EQ(v.get_preview(), "rc1");
	ASSERT_EQ(v.str(), "3.4.5-rc1");
}

TEST(Version, ParseReturnsNulloptOnInvalid)
{
	std::optional<version> parsed = version::parse("invalid");
	ASSERT_FALSE(parsed.has_value());
}

TEST(Version, CompareReleaseIsGreaterThanPreview)
{
	version release{ 1, 2, 3 };
	version preview{ 1, 2, 3, "beta" };
	ASSERT_TRUE(release > preview);
	ASSERT_TRUE(preview < release);
}

TEST(Version, CompareByMajorMinorPatch)
{
	version lower{ 1, 4, 9 };
	version higher{ 1, 5, 0 };
	ASSERT_TRUE(lower < higher);
}

TEST(Version, JsonRoundTrip)
{
	version original{ 9, 8, 7, "preview" };
	nlohmann::json j = original;
	version restored = j.get<version>();
	ASSERT_EQ(restored, original);
	ASSERT_EQ(restored.str(), "9.8.7-preview");
}
