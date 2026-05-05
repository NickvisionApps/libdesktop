#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::updates;

TEST(Updates_Test, Release_constructFromJson)
{
	nlohmann::json json
	{
		{ "url", "https://api.example.com/releases/1" },
		{ "tag_name", "v2.0.0" },
		{ "prerelease", true },
		{ "draft", false },
		{ "assets", nlohmann::json::array(
		{
			{
				{ "url", "https://api.example.com/assets/1" },
				{ "name", "app.tar.gz" },
				{ "size", 512 },
				{ "digest", "sha256:abc" },
				{ "browser_download_url", "https://download/app.tar.gz" }
			}
		}) }
	};
	release r{ json };
	EXPECT_EQ(r.get_tag_name(), "v2.0.0");
	EXPECT_TRUE(r.is_prerelease());
	EXPECT_FALSE(r.is_draft());
	ASSERT_EQ(r.get_assets().size(), 1u);
	EXPECT_EQ(r.get_assets()[0].get_name(), "app.tar.gz");
	EXPECT_FALSE(r.empty());
}

TEST(Updates_Test, Release_defaultEmpty)
{
	release r;
	EXPECT_TRUE(r.empty());
	EXPECT_FALSE(static_cast<bool>(r));
}

TEST(Updates_Test, Release_getAssets)
{
	release r;
	release_asset asset;
	asset.set_name("file.zip");
	r.set_assets({ asset });
	ASSERT_EQ(r.get_assets().size(), 1u);
	EXPECT_EQ(r.get_assets()[0].get_name(), "file.zip");
}

TEST(Updates_Test, Release_getTagName)
{
	release r;
	r.set_tag_name("v1.0.0");
	EXPECT_EQ(r.get_tag_name(), "v1.0.0");
	EXPECT_FALSE(r.empty());
	EXPECT_TRUE(static_cast<bool>(r));
}

TEST(Updates_Test, Release_isDraft)
{
	release r;
	r.set_draft(true);
	EXPECT_TRUE(r.is_draft());
	r.set_draft(false);
	EXPECT_FALSE(r.is_draft());
}

TEST(Updates_Test, Release_isPrerelease)
{
	release r;
	r.set_prerelease(true);
	EXPECT_TRUE(r.is_prerelease());
	r.set_prerelease(false);
	EXPECT_FALSE(r.is_prerelease());
}

TEST(Updates_Test, ReleaseAsset_constructFromJson)
{
	nlohmann::json json
	{
		{ "url", "https://api.example.com/assets/1" },
		{ "name", "app.tar.gz" },
		{ "size", 1024 },
		{ "digest", "sha256:abc123" },
		{ "browser_download_url", "https://download/app.tar.gz" }
	};
	release_asset asset{ json };
	EXPECT_EQ(asset.get_url(), "https://api.example.com/assets/1");
	EXPECT_EQ(asset.get_name(), "app.tar.gz");
	EXPECT_EQ(asset.get_size(), 1024);
	EXPECT_EQ(asset.get_digest(), "sha256:abc123");
	EXPECT_EQ(asset.get_browser_download_url(), "https://download/app.tar.gz");
	EXPECT_FALSE(asset.empty());
}

TEST(Updates_Test, ReleaseAsset_getBrowserDownloadUrl)
{
	release_asset asset;
	asset.set_browser_download_url("https://example.com/download");
	EXPECT_EQ(asset.get_browser_download_url(), "https://example.com/download");
}

TEST(Updates_Test, ReleaseAsset_getDigest)
{
	release_asset asset;
	asset.set_digest("sha256:abc123");
	EXPECT_EQ(asset.get_digest(), "sha256:abc123");
}

TEST(Updates_Test, ReleaseAsset_getName)
{
	release_asset asset;
	asset.set_name("my-asset.tar.gz");
	EXPECT_EQ(asset.get_name(), "my-asset.tar.gz");
}

TEST(Updates_Test, ReleaseAsset_getSize)
{
	release_asset asset;
	asset.set_size(1024);
	EXPECT_EQ(asset.get_size(), 1024);
}

TEST(Updates_Test, ReleaseAsset_getUrl)
{
	release_asset asset;
	asset.set_url("https://api.example.com/assets/1");
	EXPECT_EQ(asset.get_url(), "https://api.example.com/assets/1");
}

TEST(Updates_Test, ReleaseAsset_notEmptyWhenNamed)
{
	release_asset asset;
	asset.set_name("file.zip");
	EXPECT_FALSE(asset.empty());
	EXPECT_TRUE(static_cast<bool>(asset));
}

TEST(Updates_Test, Version_constructFromParts)
{
	version v{ 1, 2, 3 };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_FALSE(v.empty());
	EXPECT_TRUE(static_cast<bool>(v));
}

TEST(Updates_Test, Version_constructFromPartsWithPreview)
{
	version v{ 1, 2, 3, "beta" };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_EQ(v.get_preview(), "beta");
	EXPECT_TRUE(v.is_preview());
}

TEST(Updates_Test, Version_constructFromString)
{
	version v{ "1.2.3" };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_FALSE(v.is_preview());
}

TEST(Updates_Test, Version_constructFromStringWithPreview)
{
	version v{ "1.2.3-beta" };
	EXPECT_EQ(v.get_major(), 1);
	EXPECT_EQ(v.get_minor(), 2);
	EXPECT_EQ(v.get_patch(), 3);
	EXPECT_EQ(v.get_preview(), "beta");
	EXPECT_TRUE(v.is_preview());
}

TEST(Updates_Test, Version_defaultEmpty)
{
	version v;
	EXPECT_TRUE(v.empty());
	EXPECT_FALSE(static_cast<bool>(v));
}

TEST(Updates_Test, Version_equality)
{
	EXPECT_EQ(version(1, 2, 3), version(1, 2, 3));
	EXPECT_NE(version(1, 2, 3), version(1, 2, 4));
}

TEST(Updates_Test, Version_greaterThan)
{
	EXPECT_GT(version(2, 0, 0), version(1, 0, 0));
}

TEST(Updates_Test, Version_lessThan)
{
	EXPECT_LT(version(1, 0, 0), version(2, 0, 0));
	EXPECT_LT(version(1, 0, 0), version(1, 1, 0));
	EXPECT_LT(version(1, 0, 0), version(1, 0, 1));
}

TEST(Updates_Test, Version_strParts)
{
	version v{ 1, 2, 3 };
	EXPECT_EQ(v.str(), "1.2.3");
}

TEST(Updates_Test, Version_strPartsWithPreview)
{
	version v{ 1, 2, 3, "beta" };
	EXPECT_EQ(v.str(), "1.2.3-beta");
}

TEST(Updates_Test, Version_tryParse_empty)
{
	version v;
	EXPECT_FALSE(version::try_parse("", v));
}

TEST(Updates_Test, Version_tryParse_invalid)
{
	version v;
	EXPECT_FALSE(version::try_parse("not-a-version", v));
}

TEST(Updates_Test, Version_tryParse_missingPatch)
{
	version v;
	EXPECT_FALSE(version::try_parse("1.2", v));
}

TEST(Updates_Test, Version_tryParse_valid)
{
	version v;
	ASSERT_TRUE(version::try_parse("2.0.1", v));
	EXPECT_EQ(v.get_major(), 2);
	EXPECT_EQ(v.get_minor(), 0);
	EXPECT_EQ(v.get_patch(), 1);
}
