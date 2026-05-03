#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::updates;

TEST(App_Test, AppInfo_addArtist)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.add_artist("Artist One", "https://artist1.example.com");
	const std::unordered_map<std::string, std::string>& artists{ info.get_artists() };
	ASSERT_EQ(artists.size(), 1u);
	EXPECT_EQ(artists.at("Artist One"), "https://artist1.example.com");
}

TEST(App_Test, AppInfo_addDesigner)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.add_designer("Designer One", "https://designer1.example.com");
	const std::unordered_map<std::string, std::string>& designers{ info.get_designers() };
	ASSERT_EQ(designers.size(), 1u);
	EXPECT_EQ(designers.at("Designer One"), "https://designer1.example.com");
}

TEST(App_Test, AppInfo_addDeveloper)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.add_developer("Dev One", "https://dev1.example.com");
	const std::unordered_map<std::string, std::string>& devs{ info.get_developers() };
	ASSERT_EQ(devs.size(), 1u);
	EXPECT_EQ(devs.at("Dev One"), "https://dev1.example.com");
}

TEST(App_Test, AppInfo_addExtraLink)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.add_extra_link("Docs", "https://docs.example.com");
	const std::unordered_map<std::string, std::string>& links{ info.get_extra_links() };
	ASSERT_EQ(links.size(), 1u);
	EXPECT_EQ(links.at("Docs"), "https://docs.example.com");
}

TEST(App_Test, AppInfo_getChangelog)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_changelog("Initial release");
	EXPECT_EQ(info.get_changelog(), "Initial release");
}

TEST(App_Test, AppInfo_getChangelogHtml)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_changelog("Initial release");
	EXPECT_FALSE(info.get_changelog_html().empty());
}

TEST(App_Test, AppInfo_getDescription)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_description("A great app");
	EXPECT_EQ(info.get_description(), "A great app");
}

TEST(App_Test, AppInfo_getDiscussionsUrl)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_discussions_url("https://discuss.example.com");
	EXPECT_EQ(info.get_discussions_url(), "https://discuss.example.com");
}

TEST(App_Test, AppInfo_getEnglishShortName)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	EXPECT_EQ(info.get_english_short_name(), "myapp");
}

TEST(App_Test, AppInfo_getId)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	EXPECT_EQ(info.get_id(), "com.example.app");
}

TEST(App_Test, AppInfo_getIssuesUrl)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_issues_url("https://issues.example.com");
	EXPECT_EQ(info.get_issues_url(), "https://issues.example.com");
}

TEST(App_Test, AppInfo_getName)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	EXPECT_EQ(info.get_name(), "My App");
}

TEST(App_Test, AppInfo_getShortName)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_short_name("MyApp");
	EXPECT_EQ(info.get_short_name(), "MyApp");
}

TEST(App_Test, AppInfo_getSourceUrl)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_source_url("https://github.com/example/app");
	EXPECT_EQ(info.get_source_url(), "https://github.com/example/app");
}

TEST(App_Test, AppInfo_getTranslationCredits)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_translation_credits("Translator One");
	EXPECT_EQ(info.get_translation_credits(), "Translator One");
}

TEST(App_Test, AppInfo_getVersion)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	version v{ 1, 0, 0 };
	info.set_version(v);
	EXPECT_EQ(info.get_version(), v);
}

TEST(App_Test, AppInfo_isPortable)
{
	app_info info{ "com.example.app", "My App", "myapp" };
	info.set_portable(true);
	EXPECT_TRUE(info.is_portable());
	info.set_portable(false);
	EXPECT_FALSE(info.is_portable());
}

TEST(App_Test, ArgumentsService_add)
{
	int argc{ 0 };
	arguments_service svc{ argc, nullptr };
	svc.add("--verbose");
	EXPECT_TRUE(svc.contains("--verbose"));
}

TEST(App_Test, ArgumentsService_contains)
{
	int argc{ 0 };
	arguments_service svc{ argc, nullptr };
	EXPECT_FALSE(svc.contains("--missing"));
	svc.add("--present");
	EXPECT_TRUE(svc.contains("--present"));
}

TEST(App_Test, ArgumentsService_getAll)
{
	int argc{ 0 };
	arguments_service svc{ argc, nullptr };
	svc.add("--alpha");
	svc.add("--beta");
	EXPECT_EQ(svc.get_all().size(), 2u);
}

TEST(App_Test, ArgumentsService_getCount)
{
	int argc{ 0 };
	arguments_service svc{ argc, nullptr };
	EXPECT_EQ(svc.get_count(), 0u);
	svc.add("--one");
	EXPECT_EQ(svc.get_count(), 1u);
}

TEST(App_Test, ArgumentsService_getNext)
{
	int argc{ 0 };
	arguments_service svc{ argc, nullptr };
	svc.add("--output");
	svc.add("file.txt");
	std::optional<std::string> next{ svc.get_next("--output") };
	ASSERT_TRUE(next.has_value());
	EXPECT_EQ(next.value(), "file.txt");
}

TEST(App_Test, Logger_critical)
{
	logger log;
	EXPECT_NO_THROW(log.critical("This is a critical message.", __FILE__, __LINE__));
}

TEST(App_Test, Logger_debug)
{
	logger log;
	EXPECT_NO_THROW(log.debug("This is a debug message.", __FILE__, __LINE__));
}

TEST(App_Test, Logger_error)
{
	logger log;
	EXPECT_NO_THROW(log.error("This is an error message.", __FILE__, __LINE__));
}

TEST(App_Test, Logger_info)
{
	logger log;
	EXPECT_NO_THROW(log.info("This is an info message.", __FILE__, __LINE__));
}

TEST(App_Test, Logger_log)
{
	logger log;
	EXPECT_NO_THROW(log.log(log_type::info, "This is a log message.", __FILE__, __LINE__));
}

TEST(App_Test, Logger_warn)
{
	logger log;
	EXPECT_NO_THROW(log.warn("This is a warning message.", __FILE__, __LINE__));
}

TEST(App_Test, Logger_withFilePath)
{
	logger log{ "test_app.log" };
	EXPECT_NO_THROW(log.info("This is a logged-to-file message.", __FILE__, __LINE__));
}
