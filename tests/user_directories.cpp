#include <filesystem>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::filesystem;

TEST(UserDirectories, Cache)
{
	ASSERT_FALSE(user_directories::get_cache().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_cache()));
}

TEST(UserDirectories, Config)
{
	ASSERT_FALSE(user_directories::get_config().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_config()));
}

TEST(UserDirectories, Desktop)
{
	ASSERT_FALSE(user_directories::get_desktop().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_desktop()));
}

TEST(UserDirectories, Documents)
{
	ASSERT_FALSE(user_directories::get_documents().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_documents()));
}

TEST(UserDirectories, Downloads)
{
	ASSERT_FALSE(user_directories::get_downloads().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_downloads()));
}

TEST(UserDirectories, Home)
{
	ASSERT_FALSE(user_directories::get_home().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_home()));
}

TEST(UserDirectories, LocalData)
{
	ASSERT_FALSE(user_directories::get_local_data().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_local_data()));
}

TEST(UserDirectories, Music)
{
	ASSERT_FALSE(user_directories::get_music().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_music()));
}

TEST(UserDirectories, Pictures)
{
	ASSERT_FALSE(user_directories::get_pictures().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_pictures()));
}

TEST(UserDirectories, Templates)
{
	ASSERT_FALSE(user_directories::get_templates().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_templates()));
}

TEST(UserDirectories, Videos)
{
	ASSERT_FALSE(user_directories::get_videos().empty());
	ASSERT_TRUE(std::filesystem::exists(user_directories::get_videos()));
}