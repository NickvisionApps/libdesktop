#include <filesystem>
#include <sstream>
#include "app/database_service.h"
#include "filesystem/user_directories.h"
#include "system/environment.h"

using namespace desktop::filesystem;
using namespace desktop::secrets;
using namespace desktop::system;

namespace desktop::app
{
	static void bind_value(sqlite3_stmt* stmt, int index, const database_value& value)
	{
		std::visit([&](auto&& v)
		{
			using T = std::decay_t<decltype(v)>;
			if constexpr (std::is_same_v<T, int64_t>)
			{
				sqlite3_bind_int64(stmt, index, v);
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				sqlite3_bind_double(stmt, index, v);
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				sqlite3_bind_text(stmt, index, v.c_str(), -1, SQLITE_TRANSIENT);
			}
			else if constexpr (std::is_same_v<T, std::nullptr_t>)
			{
				sqlite3_bind_null(stmt, index);
			}
		}, value.get_value());
	}

	static std::vector<std::vector<database_value>> fetch_rows(sqlite3_stmt* stmt)
	{
		std::vector<std::vector<database_value>> rows;
		int col_count{ sqlite3_column_count(stmt) };
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			std::vector<database_value> row;
			for (int i = 0; i < col_count; ++i)
			{
				std::string col_name{ sqlite3_column_name(stmt, i) };
				database_value val{ col_name, nullptr };
				switch (sqlite3_column_type(stmt, i))
				{
				case SQLITE_INTEGER:
					val = { col_name, sqlite3_column_int64(stmt, i) };
					break;
				case SQLITE_FLOAT:
					val = { col_name, sqlite3_column_double(stmt, i) };
					break;
				case SQLITE_TEXT:
				{
					const char* text{ reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)) };
					val = { col_name, text ? std::string{ text } : std::string{} };
					break;
				}
				default:
					break;
				}
				row.push_back(std::move(val));
			}
			rows.push_back(std::move(row));
		}
		return rows;
	}

	static std::string quote(const std::string& name)
	{
		std::string result{ "\"" };
		for (char c : name)
		{
			if (c == '"')
			{
				result += "\"\"";
			}
			else
			{
				result += c;
			}
		}
		result += '"';
		return result;
	}

	database_service::database_service(std::shared_ptr<app_info> info, std::shared_ptr<secrets::secret_service> secret_service)
	    : m_db{ nullptr },
	      m_is_encrypted{ false },
	      m_info{ std::move(info) },
	      m_secret_service{ std::move(secret_service) }
	{
	}

	database_service::~database_service()
	{
		if (m_db != nullptr)
		{
			sqlite3_close(m_db);
			m_db = nullptr;
		}
	}

	bool database_service::is_encrypted() const
	{
		ensure_database();
		return m_is_encrypted;
	}

	bool database_service::begin_transaction()
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return false;
		}
		return sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	bool database_service::commit_transaction()
	{
		if (m_db == nullptr)
		{
			return false;
		}
		return sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	bool database_service::rollback_transaction()
	{
		if (m_db == nullptr)
		{
			return false;
		}
		return sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	bool database_service::clear_table(const std::string& table_name)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return false;
		}
		std::string sql{ "DELETE FROM " + quote(table_name) + ";" };
		return sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	int database_service::count_in_table(const std::string& table_name)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return -1;
		}
		std::string sql{ "SELECT COUNT(*) FROM " + quote(table_name) + ";" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return -1;
		}
		int count{ -1 };
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			count = sqlite3_column_int(stmt, 0);
		}
		sqlite3_finalize(stmt);
		return count;
	}

	bool database_service::contains_in_table(const std::string& table_name, const database_value& matching_value)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return false;
		}
		std::string sql{ "SELECT COUNT(*) FROM " + quote(table_name) + " WHERE " + quote(matching_value.get_column_name()) + " = ?1;" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return false;
		}
		bind_value(stmt, 1, matching_value);
		bool result{ false };
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			result = sqlite3_column_int(stmt, 0) >= 1;
		}
		sqlite3_finalize(stmt);
		return result;
	}

	bool database_service::delete_from_table(const std::string& table_name, const database_value& matching_value)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return false;
		}
		std::string sql{ "DELETE FROM " + quote(table_name) + " WHERE " + quote(matching_value.get_column_name()) + " = ?1;" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return false;
		}
		bind_value(stmt, 1, matching_value);
		bool result{ sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(m_db) > 0 };
		sqlite3_finalize(stmt);
		return result;
	}

	bool database_service::drop_table(const std::string& table_name)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return false;
		}
		std::string sql{ "DROP TABLE IF EXISTS " + quote(table_name) + ";" };
		return sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	bool database_service::ensure_table_exists(const std::string& table_name, const std::vector<std::pair<std::string, std::string>>& columns)
	{
		ensure_database();
		if (!m_db || columns.empty())
		{
			return false;
		}
		std::ostringstream defs;
		for (size_t i{ 0 }; i < columns.size(); ++i)
		{
			if (i > 0)
			{
				defs << ", ";
			}
			defs << quote(columns[i].first) << " " << columns[i].second;
		}
		std::string sql{ "CREATE TABLE IF NOT EXISTS " + quote(table_name) + " (" + defs.str() + ");" };
		return sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
	}

	int database_service::execute_non_query(const std::string& sql, const std::unordered_map<std::string, std::string>& parameters)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return -1;
		}
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return -1;
		}
		for (const std::pair<const std::string, std::string>& pair : parameters)
		{
			int idx{ sqlite3_bind_parameter_index(stmt, ("$" + pair.first).c_str()) };
			if (idx > 0)
			{
				sqlite3_bind_text(stmt, idx, pair.second.c_str(), -1, SQLITE_TRANSIENT);
			}
		}
		int rc{ sqlite3_step(stmt) };
		sqlite3_finalize(stmt);
		if (rc != SQLITE_DONE && rc != SQLITE_ROW)
		{
			return -1;
		}
		return sqlite3_changes(m_db);
	}

	std::vector<std::vector<database_value>> database_service::execute_query(const std::string& sql,
	                                                                         const std::unordered_map<std::string, std::string>& parameters)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return {};
		}
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return {};
		}
		for (const std::pair<const std::string, std::string>& pair : parameters)
		{
			int idx{ sqlite3_bind_parameter_index(stmt, ("$" + pair.first).c_str()) };
			if (idx > 0)
			{
				sqlite3_bind_text(stmt, idx, pair.second.c_str(), -1, SQLITE_TRANSIENT);
			}
		}
		std::vector<std::vector<database_value>> rows{ fetch_rows(stmt) };
		sqlite3_finalize(stmt);
		return rows;
	}

	bool database_service::insert_into_table(const std::string& table_name, const std::vector<database_value>& data)
	{
		ensure_database();
		if (!m_db || data.empty())
		{
			return false;
		}
		std::ostringstream cols;
		std::ostringstream vals;
		for (size_t i{ 0 }; i < data.size(); ++i)
		{
			if (i > 0)
			{
				cols << ", ";
				vals << ", ";
			}
			cols << quote(data[i].get_column_name());
			vals << "?" << (i + 1);
		}
		std::string sql{ "INSERT INTO " + quote(table_name) + " (" + cols.str() + ") VALUES (" + vals.str() + ");" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return false;
		}
		for (size_t i{ 0 }; i < data.size(); ++i)
		{
			bind_value(stmt, static_cast<int>(i + 1), data[i]);
		}
		const bool result{ sqlite3_step(stmt) == SQLITE_DONE };
		sqlite3_finalize(stmt);
		return result;
	}

	bool database_service::replace_into_table(const std::string& table_name, const std::vector<database_value>& data)
	{
		ensure_database();
		if (!m_db || data.empty())
		{
			return false;
		}
		std::ostringstream cols;
		std::ostringstream vals;
		for (size_t i{ 0 }; i < data.size(); ++i)
		{
			if (i > 0)
			{
				cols << ", ";
				vals << ", ";
			}
			cols << quote(data[i].get_column_name());
			vals << "?" << (i + 1);
		}
		std::string sql{ "INSERT OR REPLACE INTO " + quote(table_name) + " (" + cols.str() + ") VALUES (" + vals.str() + ");" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return false;
		}
		for (size_t i{ 0 }; i < data.size(); ++i)
		{
			bind_value(stmt, static_cast<int>(i + 1), data[i]);
		}
		bool result{ sqlite3_step(stmt) == SQLITE_DONE };
		sqlite3_finalize(stmt);
		return result;
	}

	std::vector<std::vector<database_value>> database_service::select_from_table(const std::string& table_name, const database_value& matching_value)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return {};
		}
		std::string sql{ "SELECT * FROM " + quote(table_name) + " WHERE " + quote(matching_value.get_column_name()) + " = ?1;" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return {};
		}
		bind_value(stmt, 1, matching_value);
		std::vector<std::vector<database_value>> rows{ fetch_rows(stmt) };
		sqlite3_finalize(stmt);
		return rows;
	}

	std::vector<std::vector<database_value>> database_service::select_all_from_table(const std::string& table_name)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return {};
		}
		std::string sql{ "SELECT * FROM " + quote(table_name) + ";" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return {};
		}
		std::vector<std::vector<database_value>> rows{ fetch_rows(stmt) };
		sqlite3_finalize(stmt);
		return rows;
	}

	bool database_service::table_exists(const std::string& table_name)
	{
		ensure_database();
		if (m_db == nullptr)
		{
			return false;
		}
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ?1;", -1, &stmt, nullptr) != SQLITE_OK)
		{
			return false;
		}
		sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);
		bool result{ false };
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			result = sqlite3_column_int(stmt, 0) >= 1;
		}
		sqlite3_finalize(stmt);
		return result;
	}

	bool database_service::update_in_table(const std::string& table_name, const database_value& matching_value, const std::vector<database_value>& new_data)
	{
		ensure_database();
		if (!m_db || new_data.empty())
		{
			return false;
		}
		std::ostringstream assignments;
		for (size_t i{ 0 }; i < new_data.size(); ++i)
		{
			if (i > 0)
			{
				assignments << ", ";
			}
			assignments << quote(new_data[i].get_column_name()) << " = ?" << (i + 1);
		}
		int where_idx{ static_cast<int>(new_data.size() + 1) };
		std::string sql{ "UPDATE " + quote(table_name) + " SET " + assignments.str() + " WHERE " + quote(matching_value.get_column_name()) + " = ?" +
			             std::to_string(where_idx) + ";" };
		sqlite3_stmt* stmt{ nullptr };
		if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		{
			return false;
		}
		for (size_t i{ 0 }; i < new_data.size(); ++i)
		{
			bind_value(stmt, static_cast<int>(i + 1), new_data[i]);
		}
		bind_value(stmt, where_idx, matching_value);
		bool result{ sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(m_db) > 0 };
		sqlite3_finalize(stmt);
		return result;
	}

	void database_service::ensure_database() const
	{
		std::scoped_lock lock{ m_mutex };
		if (m_db != nullptr)
		{
			return;
		}
		std::filesystem::path path;
		if (m_info->is_portable())
		{
			path = environment::get_executable_directory() / "app.db";
		}
		else
		{
			path = user_directories::get_config() / m_info->get_name() / "app.db";
		}
		std::string password;
		if (!m_info->is_portable() && m_secret_service)
		{
			try
			{
				std::optional<secret> s{ m_secret_service->get(m_info->get_id()) };
				if (!s)
				{
					s = m_secret_service->create(m_info->get_id());
				}
				if (s && !s->empty())
				{
					password = s->get_value();
				}
			}
			catch (...)
			{
			}
		}
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);
		sqlite3* db{ nullptr };
		if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
		{
			if (db)
			{
				sqlite3_close(db);
			}
			if (password.empty())
			{
				sqlite3_open_v2(":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
			}
			m_db = db;
			return;
		}
		if (!password.empty())
		{
			sqlite3_key(db, password.c_str(), static_cast<int>(password.size()));
			if (sqlite3_exec(db, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, nullptr) != SQLITE_OK)
			{
				sqlite3_close(db);
				return;
			}
			m_is_encrypted = true;
		}
		m_db = db;
	}
}