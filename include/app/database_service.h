#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <sqlite3.h>
#include "app/app_info.h"
#include "secrets/secret_service.h"
#include "services/service.h"

namespace desktop::app
{
	class database_service : public services::service
	{
	public:
		using dependencies = std::tuple<app_info, secrets::secret_service>;
		database_service(std::shared_ptr<app_info> info, std::shared_ptr<secrets::secret_service> secret_service);
		~database_service() override;
		database_service(const database_service&) = delete;
		database_service(database_service&&) = delete;
		bool is_encrypted() const;
		bool begin_transaction();
		bool commit_transaction();
		bool rollback_transaction();
		bool clear_table(const std::string& table_name);
		int count_in_table(const std::string& table_name);
		bool contains_in_table(const std::string& table_name, const std::string& column_name, const std::variant<int64_t, double, std::string, std::nullptr_t>& matching_value);
		bool delete_from_table(const std::string& table_name, const std::string& column_name, const std::variant<int64_t, double, std::string, std::nullptr_t>& matching_value);
		bool drop_table(const std::string& table_name);
		bool ensure_table_exists(const std::string& table_name, const std::string& layout);
		int execute_non_query(const std::string& sql, const std::unordered_map<std::string, std::variant<int64_t, double, std::string, std::nullptr_t>>& parameters = {});
		bool insert_into_table(const std::string& table_name, const std::unordered_map<std::string, std::variant<int64_t, double, std::string, std::nullptr_t>>& data);
		bool replace_into_table(const std::string& table_name, const std::unordered_map<std::string, std::variant<int64_t, double, std::string, std::nullptr_t>>& data);
		std::vector<std::unordered_map<std::string, std::string>> select_from_table(const std::string& table_name, const std::string& column_name, const std::variant<int64_t, double, std::string, std::nullptr_t>& matching_value);
		std::vector<std::unordered_map<std::string, std::string>> select_all_from_table(const std::string& table_name);
		bool table_exists(const std::string& table_name);
		bool update_in_table(const std::string& table_name, const std::string& column_name, const std::variant<int64_t, double, std::string, std::nullptr_t>& matching_value, const std::unordered_map<std::string, std::variant<int64_t, double, std::string, std::nullptr_t>>& new_data);
		database_service& operator=(const database_service&) = delete;
		database_service& operator=(database_service&&) = delete;

	private:
		mutable std::mutex m_mutex;
		void ensure_database();
		std::shared_ptr<app_info> m_info;
		std::shared_ptr<secrets::secret_service> m_secret_service;
		sqlite3* m_db;
		bool m_is_encrypted;
	};
}