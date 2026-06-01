#include <filesystem>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::secrets;

class DatabaseService : public testing::Test
{
protected:
	void SetUp() override
	{
		std::filesystem::path db_path{ desktop::filesystem::user_directories::get_config() / "DatabaseServiceTests" / "app.db" };
		std::error_code ec;
		std::filesystem::remove(db_path, ec);
		std::filesystem::remove(db_path.parent_path(), ec);
		std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.database", "DatabaseServiceTests", "DatabaseServiceTests", false) };
		std::shared_ptr<secret_service> secrets{ std::make_shared<secret_service>() };
		m_service = std::make_shared<database_service>(info, secrets);
	}

	void TearDown() override
	{
		m_service.reset();
		std::filesystem::path db_path{ desktop::filesystem::user_directories::get_config() / "DatabaseServiceTests" / "app.db" };
		std::error_code ec;
		std::filesystem::remove(db_path, ec);
		std::filesystem::remove(db_path.parent_path(), ec);
	}

	std::shared_ptr<database_service> m_service;
};

TEST_F(DatabaseService, NotInMemoryByDefault)
{
	ASSERT_FALSE(m_service->is_in_memory());
}

TEST_F(DatabaseService, EnsureTableExistsCreatesTable)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->table_exists("sample"));
}

TEST_F(DatabaseService, InsertAndCount)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_EQ(m_service->count_in_table("sample"), 1);
}

TEST_F(DatabaseService, ContainsInTable)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_TRUE(m_service->contains_in_table("sample", { "name", "alpha" }));
	ASSERT_FALSE(m_service->contains_in_table("sample", { "name", "beta" }));
}

TEST_F(DatabaseService, SelectFromTableReturnsMatchingRow)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	auto rows = m_service->select_from_table("sample", { "name", "alpha" });
	ASSERT_EQ(rows.size(), 1);
	ASSERT_EQ(rows[0].size(), 3);
}

TEST_F(DatabaseService, UpdateInTableChangesData)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_TRUE(m_service->update_in_table("sample", { "id", 1 }, { { "name", "beta" }, { "enabled", false } }));
	auto rows = m_service->select_from_table("sample", { "id", 1 });
	ASSERT_EQ(rows.size(), 1);
	bool found_name{ false };
	bool found_enabled{ false };
	for (database_value& col : rows[0])
	{
		if (col.get_column_name() == "name")
		{
			ASSERT_EQ(col.str(), "beta");
			found_name = true;
		}
		else if (col.get_column_name() == "enabled")
		{
			ASSERT_EQ(col.str(), "0");
			found_enabled = true;
		}
	}
	ASSERT_TRUE(found_name);
	ASSERT_TRUE(found_enabled);
}

TEST_F(DatabaseService, ReplaceIntoTableOverwritesByPrimaryKey)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_TRUE(m_service->replace_into_table("sample", { { "id", 1 }, { "name", "gamma" }, { "enabled", false } }));
	ASSERT_EQ(m_service->count_in_table("sample"), 1);
	ASSERT_TRUE(m_service->contains_in_table("sample", { "name", "gamma" }));
}

TEST_F(DatabaseService, DeleteFromTableRemovesRow)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_TRUE(m_service->delete_from_table("sample", { "id", 1 }));
	ASSERT_EQ(m_service->count_in_table("sample"), 0);
}

TEST_F(DatabaseService, ClearTableRemovesAllRows)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 2 }, { "name", "beta" }, { "enabled", false } }));
	ASSERT_TRUE(m_service->clear_table("sample"));
	ASSERT_EQ(m_service->count_in_table("sample"), 0);
}

TEST_F(DatabaseService, ExecuteNonQueryAndExecuteQueryWithParameters)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_EQ(m_service->execute_non_query("INSERT INTO sample (id, name, enabled) VALUES (1, $name, $enabled);", { { "name", "param" }, { "enabled", "1" } }),
	          1);
	auto rows = m_service->execute_query("SELECT * FROM sample WHERE name = $name;", { { "name", "param" } });
	ASSERT_EQ(rows.size(), 1);
}

TEST_F(DatabaseService, TransactionRollbackUndoesChanges)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->begin_transaction());
	ASSERT_TRUE(m_service->insert_into_table("sample", { { "id", 1 }, { "name", "alpha" }, { "enabled", true } }));
	ASSERT_TRUE(m_service->rollback_transaction());
	ASSERT_EQ(m_service->count_in_table("sample"), 0);
}

TEST_F(DatabaseService, DropTableRemovesTable)
{
	ASSERT_TRUE(m_service->ensure_table_exists("sample", { { "id", "INTEGER PRIMARY KEY" }, { "name", "TEXT" }, { "enabled", "INTEGER" } }));
	ASSERT_TRUE(m_service->drop_table("sample"));
	ASSERT_FALSE(m_service->table_exists("sample"));
}
