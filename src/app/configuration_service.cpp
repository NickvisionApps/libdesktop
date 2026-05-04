#include "app/configuration_service.h"
#include <fstream>
#include <sstream>

namespace desktop::app
{
	configuration_service::configuration_service(std::shared_ptr<database_service> db)
		: m_db{ db },
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
		std::unordered_map<std::string, std::string> result;
		for (const std::unordered_map<std::string, std::string>& row : m_db->select_all_from_table("configuration"))
		{
			if (row.contains("name") && row.contains("value"))
			{
				result[row.at("name")] = row.at("value");
			}
		}
		return result;
	}

	void configuration_service::set_many(const std::unordered_map<std::string, std::string>& values)
	{
		ensure_table();
		m_db->begin_transaction();
		for (const std::pair<const std::string, std::string>& pair : values)
		{
			m_cache[pair.first] = pair.second;
			m_db->replace_into_table("configuration",
			{ 
				{ "name", pair.first },
				{ "value", pair.second }
			});
			m_saved_event.invoke(*this, { pair.first, database_value{ pair.second } });
		}
		m_db->commit_transaction();
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
		m_db->begin_transaction();
		for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
		{
			std::string val{ it.value().is_string() ? it.value().get<std::string>() : it.value().dump() };
			m_cache[it.key()] = val;
			m_db->replace_into_table("configuration",
			{ 
				{ "name", it.key() },
				{ "value", val }
			});
			m_saved_event.invoke(*this, configuration_saved_event_args{ it.key(), database_value{ val } });
			++imported;
		}
		m_db->commit_transaction();
		return imported;
	}

	void configuration_service::ensure_table()
	{
		if (m_table_ensured)
		{
			return;
		}
		m_db->ensure_table_exists("configuration", "name TEXT PRIMARY KEY, value TEXT");
		m_table_ensured = true;
	}
}
