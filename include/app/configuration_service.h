#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "app/configuration_saved_event_args.h"
#include "app/database_service.h"
#include "events/event.h"
#include "services/service.h"

namespace desktop::app
{
	template<typename T>
	concept configuration_gettable =
		std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, int64_t> ||
		std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, std::string> ||
		std::is_enum_v<T>;

	template<typename T>
	concept configuration_settable = configuration_gettable<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string_view>;

	template<typename T>
	concept configuration_object = !configuration_settable<T>;

	class configuration_service : public services::service
	{
	public:
		using dependencies = std::tuple<database_service>;
		configuration_service(std::shared_ptr<database_service> db);
		~configuration_service() override = default;
		configuration_service(const configuration_service&) = delete;
		configuration_service(configuration_service&&) = delete;
		const events::event<configuration_service, configuration_saved_event_args>& get_saved_event() const;
		std::unordered_map<std::string, std::string> get_all_raw();
		void set_many(const std::unordered_map<std::string, std::string>& values);
		int import_from_json_file(const std::filesystem::path& path);
		template<configuration_gettable T>
		T get(const std::string& name, T default_value)
		{
			ensure_table();
			std::lock_guard<std::mutex> lock{ m_mutex };
			std::string raw;
			bool found{ false };
			if (m_cache.contains(name))
			{
				raw = m_cache.at(name);
				found = true;
			}
			else
			{
				std::vector<std::unordered_map<std::string, std::string>> rows{ m_db->select_from_table("configuration", "name", name) };
				if (!rows.empty() && rows[0].contains("value"))
				{
					raw = rows[0].at("value");
					m_cache[name] = raw;
					found = true;
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
		template<configuration_object T>
		T get(const std::string& name, const T& default_value)
		{
			ensure_table();
			std::lock_guard<std::mutex> lock{ m_mutex };
			std::string raw;
			if (m_cache.contains(name))
			{
				raw = m_cache.at(name);
			}
			else
			{
				std::vector<std::unordered_map<std::string, std::string>> rows{ m_db->select_from_table("configuration", "name", name) };
				if (rows.empty() || !rows[0].contains("value"))
				{
					return default_value;
				}
				raw = rows[0].at("value");
				m_cache[name] = raw;
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
		template<configuration_settable T>
		void set(const std::string& name, T value)
		{
			std::string str_value;
			database_value event_value{ nullptr };
			if constexpr (std::is_same_v<T, bool>)
			{
				str_value = value ? "true" : "false";
				event_value = database_value{ static_cast<int64_t>(value ? 1 : 0) };
			}
			else if constexpr (std::is_same_v<T, int>)
			{
				str_value = std::to_string(value);
				event_value = database_value{ static_cast<int64_t>(value) };
			}
			else if constexpr (std::is_same_v<T, int64_t>)
			{
				str_value = std::to_string(value);
				event_value = database_value{ value };
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				str_value = std::to_string(value);
				event_value = database_value{ static_cast<double>(value) };
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				str_value = std::to_string(value);
				event_value = database_value{ value };
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				str_value = value;
				event_value = database_value{ value };
			}
			else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, std::string_view>)
			{
				str_value = std::string(value);
				event_value = database_value{ value };
			}
			else
			{
				str_value = std::to_string(static_cast<int64_t>(value));
				event_value = database_value{ static_cast<int64_t>(value) };
			}
			ensure_table();
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_cache[name] = str_value;
			m_db->replace_into_table("configuration",
			{ 
				{ "name", name },
				{ "value", str_value }
			});
			lock.unlock();
			m_saved_event.invoke(*this, { name, event_value });
		}
		template<configuration_object T>
		void set(const std::string& name, const T& value)
		{
			std::string json_str{ nlohmann::json(value).dump() };
			ensure_table();
			std::unique_lock<std::mutex> lock{ m_mutex };
			m_cache[name] = json_str;
			m_db->replace_into_table("configuration",
			{ 
				{ "name", name },
				{ "value", json_str }
			});
			lock.unlock();
			m_saved_event.invoke(*this, { name, json_str });
		}
		configuration_service& operator=(const configuration_service&) = delete;
		configuration_service& operator=(configuration_service&&) = delete;

	private:
		void ensure_table();
		mutable std::mutex m_mutex;
		std::shared_ptr<database_service> m_db;
		bool m_table_ensured;
		std::unordered_map<std::string, std::string> m_cache;
		events::event<configuration_service, configuration_saved_event_args> m_saved_event;
	};
}
