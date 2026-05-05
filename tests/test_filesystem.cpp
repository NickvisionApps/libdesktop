#include <filesystem>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::filesystem;

TEST(Filesystem_Test, UserDirectories_getCache)
{
	EXPECT_FALSE(user_directories::get_cache().empty());
}

TEST(Filesystem_Test, UserDirectories_getCacheIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_cache() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getConfig)
{
	EXPECT_FALSE(user_directories::get_config().empty());
}

TEST(Filesystem_Test, UserDirectories_getConfigIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_config() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getDesktop)
{
	EXPECT_FALSE(user_directories::get_desktop().empty());
}

TEST(Filesystem_Test, UserDirectories_getDesktopIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_desktop() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getDocuments)
{
	EXPECT_FALSE(user_directories::get_documents().empty());
}

TEST(Filesystem_Test, UserDirectories_getDocumentsIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_documents() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getDownloads)
{
	EXPECT_FALSE(user_directories::get_downloads().empty());
}

TEST(Filesystem_Test, UserDirectories_getDownloadsIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_downloads() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getHome)
{
	EXPECT_FALSE(user_directories::get_home().empty());
}

TEST(Filesystem_Test, UserDirectories_getHomeIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_home() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getLocalData)
{
	EXPECT_FALSE(user_directories::get_local_data().empty());
}

TEST(Filesystem_Test, UserDirectories_getLocalDataIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_local_data() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getMusic)
{
	EXPECT_FALSE(user_directories::get_music().empty());
}

TEST(Filesystem_Test, UserDirectories_getMusicIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_music() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getPictures)
{
	EXPECT_FALSE(user_directories::get_pictures().empty());
}

TEST(Filesystem_Test, UserDirectories_getPicturesIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_pictures() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getTemplates)
{
	EXPECT_FALSE(user_directories::get_templates().empty());
}

TEST(Filesystem_Test, UserDirectories_getTemplatesIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_templates() }.is_absolute());
}

TEST(Filesystem_Test, UserDirectories_getVideos)
{
	EXPECT_FALSE(user_directories::get_videos().empty());
}

TEST(Filesystem_Test, UserDirectories_getVideosIsAbsolute)
{
	EXPECT_TRUE(std::filesystem::path{ user_directories::get_videos() }.is_absolute());
}
