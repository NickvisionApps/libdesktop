#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::filesystem;
using namespace desktop::secrets;

class DatabaseService_Test : public ::testing::Test
{
public:
	DatabaseService_Test()
	    : m_info{ std::make_shared<app_info>("com.example.test", "TestApp", "testapp") },
	      m_secret_svc{ std::make_shared<secret_service>() },
	      m_svc{ m_info, m_secret_svc }
	{
		if (!m_removed)
		{
			try
			{
				std::filesystem::remove(user_directories::get_config() / m_info->get_name() / "app.db");
			}
			catch (...)
			{
			}
			m_removed = true;
		}
	}

protected:
	std::shared_ptr<app_info> m_info;
	std::shared_ptr<secret_service> m_secret_svc;
	database_service m_svc;

private:
	static bool m_removed;
};

bool DatabaseService_Test::m_removed = false;

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
	m_svc.ensure_table_exists("clearTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	m_svc.insert_into_table("clearTable", { { "name", "Alice" } });
	EXPECT_TRUE(m_svc.clear_table("clearTable"));
	EXPECT_EQ(m_svc.count_in_table("clearTable"), 0);
	m_svc.drop_table("clearTable");
}

TEST_F(DatabaseService_Test, DatabaseService_containsInTable)
{
	m_svc.ensure_table_exists("containsInTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	m_svc.insert_into_table("containsInTable", { { "name", "Bob" } });
	EXPECT_TRUE(m_svc.contains_in_table("containsInTable", { "name", "Bob" }));
	EXPECT_FALSE(m_svc.contains_in_table("containsInTable", { "name", "Charlie" }));
	m_svc.drop_table("containsInTable");
}

TEST_F(DatabaseService_Test, DatabaseService_countInTable)
{
	m_svc.ensure_table_exists("countInTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	EXPECT_EQ(m_svc.count_in_table("countInTable"), 0);
	m_svc.insert_into_table("countInTable", { { "name", "Alice" } });
	m_svc.insert_into_table("countInTable", { { "name", "Bob" } });
	EXPECT_EQ(m_svc.count_in_table("countInTable"), 2);
	m_svc.drop_table("countInTable");
}

TEST_F(DatabaseService_Test, DatabaseService_deleteFromTable)
{
	m_svc.ensure_table_exists("deleteFromTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	m_svc.insert_into_table("deleteFromTable", { { "name", "Alice" } });
	EXPECT_TRUE(m_svc.delete_from_table("deleteFromTable", { "name", "Alice" }));
	EXPECT_EQ(m_svc.count_in_table("deleteFromTable"), 0);
	m_svc.drop_table("deleteFromTable");
}

TEST_F(DatabaseService_Test, DatabaseService_dropTable)
{
	m_svc.ensure_table_exists("dropTable", { { "id", "INTEGER PRIMARY KEY" } });
	EXPECT_TRUE(m_svc.table_exists("dropTable"));
	EXPECT_TRUE(m_svc.drop_table("dropTable"));
	EXPECT_FALSE(m_svc.table_exists("dropTable"));
}

TEST_F(DatabaseService_Test, DatabaseService_ensureTableExists)
{
	EXPECT_TRUE(m_svc.ensure_table_exists("ensureTableExists", { { "id", "INTEGER PRIMARY KEY" }, { "val", "TEXT" } }));
	EXPECT_TRUE(m_svc.table_exists("ensureTableExists"));
	m_svc.drop_table("ensureTableExists");
}

TEST_F(DatabaseService_Test, DatabaseService_executeNonQuery)
{
	m_svc.ensure_table_exists("executeNonQuery", { { "id", "INTEGER PRIMARY KEY" }, { "val", "TEXT" } });
	int affected{ m_svc.execute_non_query("INSERT INTO executeNonQuery (val) VALUES ($val);", { { "val", "hello" } }) };
	EXPECT_EQ(affected, 1);
	m_svc.drop_table("executeNonQuery");
}

TEST_F(DatabaseService_Test, DatabaseService_executeQuery)
{
	m_svc.ensure_table_exists("executeQuery", { { "id", "INTEGER PRIMARY KEY" }, { "val", "TEXT" } });
	m_svc.insert_into_table("executeQuery", { { "val", "alpha" } });
	m_svc.insert_into_table("executeQuery", { { "val", "beta" } });
	std::vector<std::vector<database_value>> rows{ m_svc.execute_query("SELECT * FROM executeQuery WHERE val = $val;", { { "val", "alpha" } }) };
	ASSERT_EQ(rows.size(), 1u);
	std::string val;
	for (const database_value& col : rows[0])
	{
		if (col.get_column_name() == "val")
		{
			val = col.str();
			break;
		}
	}
	EXPECT_EQ(val, "alpha");
	m_svc.drop_table("executeQuery");
}

TEST_F(DatabaseService_Test, DatabaseService_insertIntoTable)
{
	m_svc.ensure_table_exists("insertIntoTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	EXPECT_TRUE(m_svc.insert_into_table("insertIntoTable", { { "name", "Alice" } }));
	EXPECT_EQ(m_svc.count_in_table("insertIntoTable"), 1);
	m_svc.drop_table("insertIntoTable");
}

TEST_F(DatabaseService_Test, DatabaseService_isEncrypted)
{
	EXPECT_TRUE(m_svc.is_encrypted());
}

TEST_F(DatabaseService_Test, DatabaseService_replaceIntoTable)
{
	m_svc.ensure_table_exists("replaceIntoTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT UNIQUE" } });
	m_svc.insert_into_table("replaceIntoTable", { { "name", "Alice" } });
	EXPECT_TRUE(m_svc.replace_into_table("replaceIntoTable", { { "name", "Alice" } }));
	EXPECT_EQ(m_svc.count_in_table("replaceIntoTable"), 1);
	m_svc.drop_table("replaceIntoTable");
}

TEST_F(DatabaseService_Test, DatabaseService_selectAllFromTable)
{
	m_svc.ensure_table_exists("selectAllFromTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	m_svc.insert_into_table("selectAllFromTable", { { "name", "Alice" } });
	m_svc.insert_into_table("selectAllFromTable", { { "name", "Bob" } });
	std::vector<std::vector<database_value>> rows{ m_svc.select_all_from_table("selectAllFromTable") };
	EXPECT_EQ(rows.size(), 2u);
	m_svc.drop_table("selectAllFromTable");
}

TEST_F(DatabaseService_Test, DatabaseService_selectFromTable)
{
	m_svc.ensure_table_exists("selectFromTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	m_svc.insert_into_table("selectFromTable", { { "name", "Alice" } });
	m_svc.insert_into_table("selectFromTable", { { "name", "Bob" } });
	std::vector<std::vector<database_value>> rows{ m_svc.select_from_table("selectFromTable", { "name", "Alice" }) };
	ASSERT_EQ(rows.size(), 1u);
	std::string name_val;
	for (const database_value& col : rows[0])
	{
		if (col.get_column_name() == "name")
		{
			name_val = col.str();
			break;
		}
	}
	EXPECT_EQ(name_val, "Alice");
	m_svc.drop_table("selectFromTable");
}

TEST_F(DatabaseService_Test, DatabaseService_tableExists)
{
	EXPECT_FALSE(m_svc.table_exists("tableExists"));
	m_svc.ensure_table_exists("tableExists", { { "id", "INTEGER PRIMARY KEY" } });
	EXPECT_TRUE(m_svc.table_exists("tableExists"));
	m_svc.drop_table("tableExists");
}

TEST_F(DatabaseService_Test, DatabaseService_updateInTable)
{
	m_svc.ensure_table_exists("updateInTable", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" } });
	m_svc.insert_into_table("updateInTable", { { "name", "Alice" } });
	EXPECT_TRUE(m_svc.update_in_table("updateInTable", { "name", "Alice" }, { { "name", "Alicia" } }));
	std::vector<std::vector<database_value>> rows{ m_svc.select_from_table("updateInTable", { "name", "Alicia" }) };
	ASSERT_EQ(rows.size(), 1u);
	std::string name_val;
	for (const database_value& col : rows[0])
	{
		if (col.get_column_name() == "name")
		{
			name_val = col.str();
			break;
		}
	}
	EXPECT_EQ(name_val, "Alicia");
	m_svc.drop_table("updateInTable");
}
