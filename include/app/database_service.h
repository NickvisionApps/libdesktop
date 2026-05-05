#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <sqlite3.h>
#include "app/app_info.h"
#include "app/database_value.h"
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
		bool contains_in_table(const std::string& table_name, const database_value& matching_value);
		bool delete_from_table(const std::string& table_name, const database_value& matching_value);
		bool drop_table(const std::string& table_name);
		bool ensure_table_exists(const std::string& table_name, const std::string& layout);
		int execute_non_query(const std::string& sql, const std::unordered_map<std::string, std::string>& parameters = {});
		std::vector<std::vector<database_value>> execute_query(const std::string& sql, const std::unordered_map<std::string, std::string>& parameters = {});
		bool insert_into_table(const std::string& table_name, const std::vector<database_value>& data);
		bool replace_into_table(const std::string& table_name, const std::vector<database_value>& data);
		std::vector<std::vector<database_value>> select_from_table(const std::string& table_name, const database_value& matching_value);
		std::vector<std::vector<database_value>> select_all_from_table(const std::string& table_name);
		bool table_exists(const std::string& table_name);
		bool update_in_table(const std::string& table_name, const database_value& matching_value, const std::vector<database_value>& new_data);
		database_service& operator=(const database_service&) = delete;
		database_service& operator=(database_service&&) = delete;

	private:
		void ensure_database() const;
		mutable std::mutex m_mutex;
		mutable sqlite3* m_db;
		mutable bool m_is_encrypted;
		std::shared_ptr<app_info> m_info;
		std::shared_ptr<secrets::secret_service> m_secret_service;
	};
}