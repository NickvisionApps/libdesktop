#include "app/configuration_service.h"
#include <fstream>
#include <sstream>

using namespace desktop::database;

namespace desktop::app
{
	configuration_service::configuration_service(std::shared_ptr<database_service> db)
	    : m_db{ std::move(db) },
	      m_table_ensured{ false }
	{
	}

	const events::event<configuration_service, configuration_saved_event_args>& configuration_service::get_saved_event() const
	{
		return m_saved_event;
	}

	std::unordered_map<std::string, std::string> configuration_service::get_all_raw()
	{
		ensure_table();
		std::scoped_lock lock{ m_mutex };
		std::unordered_map<std::string, std::string> result;
		for (const std::vector<database_value>& row : m_db->select_all_from_table("configuration"))
		{
			std::string name;
			std::string value;
			bool name_found{ false };
			for (const database_value& col : row)
			{
				if (col.get_column_name() == "name")
				{
					name = col.str();
					name_found = true;
				}
				else if (col.get_column_name() == "value")
				{
					value = col.str();
				}
			}
			if (name_found)
			{
				result[name] = value;
				m_cache[name] = value;
			}
		}
		return result;
	}

	int configuration_service::import_from_json_file(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			return 0;
		}
		std::ifstream file{ path };
		if (!file.is_open())
		{
			return 0;
		}
		std::ostringstream oss;
		oss << file.rdbuf();
		nlohmann::json j;
		try
		{
			j = nlohmann::json::parse(oss.str());
		}
		catch (...)
		{
			return 0;
		}
		int imported{ 0 };
		ensure_table();
		std::unique_lock<std::mutex> lock{ m_mutex };
		m_db->begin_transaction();
		for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
		{
			std::string val{ it.value().is_string() ? it.value().get<std::string>() : it.value().dump() };
			m_cache[it.key()] = val;
			m_db->replace_into_table("configuration", { { "name", it.key() }, { "value", val } });
			imported++;
		}
		m_db->commit_transaction();
		lock.unlock();
		m_saved_event.invoke(*this, {});
		return imported;
	}

	void configuration_service::ensure_table()
	{
		if (m_table_ensured)
		{
			return;
		}
		std::scoped_lock lock{ m_mutex };
		if (m_table_ensured)
		{
			return;
		}
		m_db->ensure_table_exists("configuration", { { "name", "TEXT PRIMARY KEY" }, { "value", "TEXT" } });
		m_table_ensured = true;
	}
}
