#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::updates;

TEST(UpdatesTest, Version_DefaultEmpty)
{
	version v;
	EXPECT_TRUE(v.empty());
	EXPECT_FALSE(static_cast<bool>(v));
}

TEST(UpdatesTest, Version_ConstructFromParts)
{
	version v{ 1, 2, 3 };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_FALSE(v.empty());
	EXPECT_TRUE(static_cast<bool>(v));
}

TEST(UpdatesTest, Version_ConstructFromPartsWithPreview)
{
	version v{ 1, 2, 3, "beta" };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_EQ(v.get_preview(), "beta");
	EXPECT_TRUE(v.is_preview());
}

TEST(UpdatesTest, Version_ConstructFromString)
{
	version v{ "1.2.3" };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_FALSE(v.is_preview());
}

TEST(UpdatesTest, Version_ConstructFromStringWithPreview)
{
	version v{ "1.2.3-beta" };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_EQ(v.get_preview(), "beta");
	EXPECT_TRUE(v.is_preview());
}

TEST(UpdatesTest, Version_Str_Parts)
{
	version v{ 1, 2, 3 };
	EXPECT_EQ(v.str(), "1.2.3");
}

TEST(UpdatesTest, Version_Str_PartsWithPreview)
{
	version v{ 1, 2, 3, "beta" };
	EXPECT_EQ(v.str(), "1.2.3-beta");
}

TEST(UpdatesTest, Version_TryParse_Valid)
{
	version v;
	ASSERT_TRUE(version::try_parse("2.0.1", v));
	EXPECT_EQ(v.get_major(), 2);
	EXPECT_EQ(v.get_minor(), 0);
	EXPECT_EQ(v.get_patch(), 1);
}

TEST(UpdatesTest, Version_TryParse_Invalid)
{
	version v;
	EXPECT_FALSE(version::try_parse("not-a-version", v));
}

TEST(UpdatesTest, Version_Equality)
{
	EXPECT_EQ(version(1, 2, 3), version(1, 2, 3));
	EXPECT_NE(version(1, 2, 3), version(1, 2, 4));
}

TEST(UpdatesTest, Version_LessThan)
{
	EXPECT_LT(version(1, 0, 0), version(2, 0, 0));
	EXPECT_LT(version(1, 0, 0), version(1, 1, 0));
	EXPECT_LT(version(1, 0, 0), version(1, 0, 1));
}

TEST(UpdatesTest, Version_GreaterThan)
{
	EXPECT_GT(version(2, 0, 0), version(1, 0, 0));
}
