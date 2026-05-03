#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::updates;

class DatabaseService_Test : public ::testing::Test
{
protected:
	std::shared_ptr<app_info> m_info;
	database_service m_svc;

	DatabaseService_Test()
		: m_info{ std::make_shared<app_info>("com.example.test", "TestApp", "testapp") }
		, m_svc{ m_info, nullptr }
	{
		m_info->set_portable(true);
		try
		{
			std::filesystem::remove("app.db");
		}
		catch (...) { }
	}

	void TearDown() override
	{
		try
		{
			std::filesystem::remove("app.db");
		}
		catch (...) {}
	}
};

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

TEST_F(DatabaseService_Test, DatabaseService_beginCommitTransaction)
{
	EXPECT_TRUE(m_svc.begin_transaction());
	EXPECT_TRUE(m_svc.commit_transaction());
}

TEST_F(DatabaseService_Test, DatabaseService_beginRollbackTransaction)
{
	EXPECT_TRUE(m_svc.begin_transaction());
	EXPECT_TRUE(m_svc.rollback_transaction());
}

TEST_F(DatabaseService_Test, DatabaseService_clearTable)
{
	m_svc.ensure_table_exists("clearTable", "id INTEGER PRIMARY KEY, name TEXT");
	m_svc.insert_into_table("clearTable",
	{ 
		{ "name", std::string{ "Alice" } }
	});
	EXPECT_TRUE(m_svc.clear_table("clearTable"));
	EXPECT_EQ(m_svc.count_in_table("clearTable"), 0);
	m_svc.drop_table("clearTable");
}

TEST_F(DatabaseService_Test, DatabaseService_containsInTable)
{
	m_svc.ensure_table_exists("containsInTable", "id INTEGER PRIMARY KEY, name TEXT");
	m_svc.insert_into_table("containsInTable",
	{ 
		{ "name", std::string{ "Bob" } } 
	});
	EXPECT_TRUE(m_svc.contains_in_table("containsInTable", "name", std::string{ "Bob" }));
	EXPECT_FALSE(m_svc.contains_in_table("containsInTable", "name", std::string{ "Charlie" }));
	m_svc.drop_table("containsInTable");
}

TEST_F(DatabaseService_Test, DatabaseService_countInTable)
{
	m_svc.ensure_table_exists("countInTable", "id INTEGER PRIMARY KEY, name TEXT");
	EXPECT_EQ(m_svc.count_in_table("countInTable"), 0);
	m_svc.insert_into_table("countInTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	});
	m_svc.insert_into_table("countInTable",
	{ 
		{ "name", std::string{ "Bob" } } 
	});
	EXPECT_EQ(m_svc.count_in_table("countInTable"), 2);
	m_svc.drop_table("countInTable");
}

TEST_F(DatabaseService_Test, DatabaseService_deleteFromTable)
{
	m_svc.ensure_table_exists("deleteFromTable", "id INTEGER PRIMARY KEY, name TEXT");
	m_svc.insert_into_table("deleteFromTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	});
	EXPECT_TRUE(m_svc.delete_from_table("deleteFromTable", "name", std::string{ "Alice" }));
	EXPECT_EQ(m_svc.count_in_table("deleteFromTable"), 0);
	m_svc.drop_table("deleteFromTable");
}

TEST_F(DatabaseService_Test, DatabaseService_dropTable)
{
	m_svc.ensure_table_exists("dropTable", "id INTEGER PRIMARY KEY");
	EXPECT_TRUE(m_svc.table_exists("dropTable"));
	EXPECT_TRUE(m_svc.drop_table("dropTable"));
	EXPECT_FALSE(m_svc.table_exists("dropTable"));
}

TEST_F(DatabaseService_Test, DatabaseService_ensureTableExists)
{
	EXPECT_TRUE(m_svc.ensure_table_exists("ensureTableExists", "id INTEGER PRIMARY KEY, val TEXT"));
	EXPECT_TRUE(m_svc.table_exists("ensureTableExists"));
	m_svc.drop_table("ensureTableExists");
}

TEST_F(DatabaseService_Test, DatabaseService_executeNonQuery)
{
	m_svc.ensure_table_exists("executeNonQuery", "id INTEGER PRIMARY KEY, val TEXT");
	int affected{ m_svc.execute_non_query("INSERT INTO executeNonQuery (val) VALUES ($val);",
	{ 
		{ "val", std::string{ "hello" } } 
	}) };
	EXPECT_EQ(affected, 1);
	m_svc.drop_table("executeNonQuery");
}

TEST_F(DatabaseService_Test, DatabaseService_insertIntoTable)
{
	m_svc.ensure_table_exists("insertIntoTable", "id INTEGER PRIMARY KEY, name TEXT");
	EXPECT_TRUE(m_svc.insert_into_table("insertIntoTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	}));
	EXPECT_EQ(m_svc.count_in_table("insertIntoTable"), 1);
	m_svc.drop_table("insertIntoTable");
}

TEST_F(DatabaseService_Test, DatabaseService_isEncrypted)
{
	EXPECT_FALSE(m_svc.is_encrypted());
}

TEST_F(DatabaseService_Test, DatabaseService_replaceIntoTable)
{
	m_svc.ensure_table_exists("replaceIntoTable", "id INTEGER PRIMARY KEY, name TEXT UNIQUE");
	m_svc.insert_into_table("replaceIntoTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	});
	EXPECT_TRUE(m_svc.replace_into_table("replaceIntoTable",
	{ 
		{ "name", std::string{ "Alice" } }
	}));
	EXPECT_EQ(m_svc.count_in_table("replaceIntoTable"), 1);
	m_svc.drop_table("replaceIntoTable");
}

TEST_F(DatabaseService_Test, DatabaseService_selectAllFromTable)
{
	m_svc.ensure_table_exists("selectAllFromTable", "id INTEGER PRIMARY KEY, name TEXT");
	m_svc.insert_into_table("selectAllFromTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	});
	m_svc.insert_into_table("selectAllFromTable",
	{ 
		{ "name", std::string{ "Bob" } } 
	});
	std::vector<std::unordered_map<std::string, std::string>> rows{ m_svc.select_all_from_table("selectAllFromTable") };
	EXPECT_EQ(rows.size(), 2u);
	m_svc.drop_table("selectAllFromTable");
}

TEST_F(DatabaseService_Test, DatabaseService_selectFromTable)
{
	m_svc.ensure_table_exists("selectFromTable", "id INTEGER PRIMARY KEY, name TEXT");
	m_svc.insert_into_table("selectFromTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	});
	m_svc.insert_into_table("selectFromTable",
	{ 
		{ "name", std::string{ "Bob" } } 
	});
	std::vector<std::unordered_map<std::string, std::string>> rows{ m_svc.select_from_table("selectFromTable", "name", std::string{ "Alice" }) };
	ASSERT_EQ(rows.size(), 1u);
	EXPECT_EQ(rows[0].at("name"), "Alice");
	m_svc.drop_table("selectFromTable");
}

TEST_F(DatabaseService_Test, DatabaseService_tableExists)
{
	EXPECT_FALSE(m_svc.table_exists("tableExists"));
	m_svc.ensure_table_exists("tableExists", "id INTEGER PRIMARY KEY");
	EXPECT_TRUE(m_svc.table_exists("tableExists"));
	m_svc.drop_table("tableExists");
}

TEST_F(DatabaseService_Test, DatabaseService_updateInTable)
{
	m_svc.ensure_table_exists("updateInTable", "id INTEGER PRIMARY KEY, name TEXT");
	m_svc.insert_into_table("updateInTable",
	{ 
		{ "name", std::string{ "Alice" } } 
	});
	EXPECT_TRUE(m_svc.update_in_table("updateInTable", "name", std::string{ "Alice" },
	{ 
		{ "name", std::string{ "Alicia" } } }
	));
	std::vector<std::unordered_map<std::string, std::string>> rows{ m_svc.select_from_table("updateInTable", "name", std::string{ "Alicia" }) };
	ASSERT_EQ(rows.size(), 1u);
	EXPECT_EQ(rows[0].at("name"), "Alicia");
	m_svc.drop_table("updateInTable");
}
