#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "app/configuration_saved_event_args.h"
#include "database/database_service.h"
#include "events/event.h"

namespace desktop::app
{
	template <typename T>
	concept configuration_gettable = std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, int64_t> || std::is_same_v<T, float> ||
	                                 std::is_same_v<T, double> || std::is_same_v<T, std::string> || std::is_enum_v<T>;

	template <typename T>
	concept configuration_settable = configuration_gettable<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string_view>;

	template <typename T>
	concept configuration_object = !configuration_settable<T>;

	class configuration_service
	{
	public:
		using dependencies = std::tuple<database::database_service>;
		configuration_service(std::shared_ptr<database::database_service> db);
		~configuration_service() = default;
		configuration_service(const configuration_service&) = delete;
		configuration_service(configuration_service&&) = delete;
		const events::event<configuration_service, configuration_saved_event_args>& get_saved_event() const;
		std::unordered_map<std::string, std::string> get_all_raw();
		int import_from_json_file(const std::filesystem::path& path);
		template <configuration_gettable T>
		T get(const std::string& name, T default_value)
		{
			ensure_table();
			std::scoped_lock lock{ m_mutex };
			std::string raw;
			bool found{ false };
			if (m_cache.contains(name))
			{
				raw = m_cache.at(name);
				found = true;
			}
			else
			{
				std::vector<std::vector<database::database_value>> rows{ m_db->select_from_table("configuration", { "name", name }) };
				if (!rows.empty())
				{
					for (const database::database_value& col : rows[0])
					{
						if (col.get_column_name() == "value")
						{
							raw = col.str();
							m_cache[name] = raw;
							found = true;
							break;
						}
					}
				}
			}
			if (!found)
			{
				return default_value;
			}
			try
			{
				if constexpr (std::is_same_v<T, bool>)
				{
					return raw == "true" || raw == "1";
				}
				else if constexpr (std::is_same_v<T, int>)
				{
					return std::stoi(raw);
				}
				else if constexpr (std::is_same_v<T, int64_t>)
				{
					return std::stoll(raw);
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					return std::stof(raw);
				}
				else if constexpr (std::is_same_v<T, double>)
				{
					return std::stod(raw);
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					return raw;
				}
				else
				{
					return static_cast<T>(std::stoll(raw));
				}
			}
			catch (...)
			{
				return default_value;
			}
		}
		template <configuration_object T>
		T get(const std::string& name, const T& default_value)
		{
			ensure_table();
			std::scoped_lock lock{ m_mutex };
			std::string raw;
			if (m_cache.contains(name))
			{
				raw = m_cache.at(name);
			}
			else
			{
				std::vector<std::vector<database::database_value>> rows{ m_db->select_from_table("configuration", database::database_value{ "name", name }) };
				if (rows.empty())
				{
					return default_value;
				}
				bool value_found{ false };
				for (const database::database_value& col : rows[0])
				{
					if (col.get_column_name() == "value")
					{
						raw = col.str();
						m_cache[name] = raw;
						value_found = true;
						break;
					}
				}
				if (!value_found)
				{
					return default_value;
				}
			}
			try
			{
				return nlohmann::json::parse(raw).get<T>();
			}
			catch (...)
			{
				return default_value;
			}
		}
		template <configuration_settable T>
		void set(const std::string& name, T value)
		{
			database::database_value db_value{ "value", value };
			ensure_table();
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_cache[name] = db_value.str();
			m_db->replace_into_table("configuration", { { "name", name }, db_value });
			lock.unlock();
			m_saved_event.invoke(*this, { name, db_value });
		}
		template <configuration_object T>
		void set(const std::string& name, const T& value)
		{
			std::string json_str{ nlohmann::json(value).dump() };
			database::database_value db_value{ "value", json_str };
			ensure_table();
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_cache[name] = db_value.str();
			m_db->replace_into_table("configuration", { { "name", name }, db_value });
			lock.unlock();
			m_saved_event.invoke(*this, { name, db_value });
		}
		configuration_service& operator=(const configuration_service&) = delete;
		configuration_service& operator=(configuration_service&&) = delete;

	private:
		void ensure_table();
		mutable std::mutex m_mutex;
		std::shared_ptr<database::database_service> m_db;
		std::atomic<bool> m_table_ensured;
		std::unordered_map<std::string, std::string> m_cache;
		events::event<configuration_service, configuration_saved_event_args> m_saved_event;
	};
}
