#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::filesystem;

TEST(FilesystemTest, CacheDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_cache().empty());
}

TEST(FilesystemTest, ConfigDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_config().empty());
}

TEST(FilesystemTest, DesktopDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_desktop().empty());
}

TEST(FilesystemTest, DocumentsDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_documents().empty());
}

TEST(FilesystemTest, DownloadsDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_downloads().empty());
}

TEST(FilesystemTest, HomeDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_home().empty());
}

TEST(FilesystemTest, LocalDataDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_local_data().empty());
}

TEST(FilesystemTest, MusicDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_music().empty());
}

TEST(FilesystemTest, PicturesDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_pictures().empty());
}

TEST(FilesystemTest, TemplatesDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_templates().empty());
}

TEST(FilesystemTest, VideosDirectoryNotEmpty)
{
	EXPECT_FALSE(user_directories::get_videos().empty());
}
