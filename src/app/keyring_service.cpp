#include "app/keyring_service.h"
#include <algorithm>
#include <ranges>

using namespace desktop::database;
using namespace desktop::secrets;

namespace desktop::app
{
	keyring_service::keyring_service(std::shared_ptr<database_service> db)
	    : m_db{ std::move(db) },
	      m_table_ensured{ false }
	{
	}

	bool keyring_service::add_credential(const credential& credential)
	{
		ensure_table();
		std::scoped_lock lock{ m_mutex };
		if (std::ranges::find(m_credentials, credential) != m_credentials.end())
		{
			return false;
		}
		if (m_db->insert_into_table("credentials", { { "name", credential.get_name() },
		                                             { "username", credential.get_username() },
		                                             { "password", credential.get_password() },
		                                             { "uri", credential.get_url() } }))
		{
			m_credentials.push_back(credential);
			return true;
		}
		return false;
	}

	bool keyring_service::delete_credential(const credential& credential)
	{
		ensure_table();
		std::scoped_lock lock{ m_mutex };
		if (m_db->delete_from_table("credentials", { "name", credential.get_name() }))
		{
			std::erase(m_credentials, credential);
			return true;
		}
		return false;
	}

	const std::vector<credential>& keyring_service::get_all_credentials()
	{
		ensure_table();
		return m_credentials;
	}

	bool keyring_service::update_credential(const credential& credential)
	{
		ensure_table();
		std::scoped_lock lock{ m_mutex };
		std::vector<secrets::credential>::const_iterator it{ std::ranges::find(m_credentials, credential) };
		if (it == m_credentials.end())
		{
			return false;
		}
		if (m_db->update_in_table("credentials", { "name", credential.get_name() },
		                          { { "username", credential.get_username() }, { "password", credential.get_password() }, { "uri", credential.get_url() } }))
		{
			m_credentials[std::distance(m_credentials.cbegin(), it)] = credential;
			return true;
		}
		return false;
	}

	void keyring_service::ensure_table()
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
		m_db->ensure_table_exists("credentials", { { "name", "TEXT PRIMARY KEY" }, { "uri", "TEXT" }, { "username", "TEXT" }, { "password", "TEXT" } });
		for (const std::vector<database_value>& row : m_db->select_all_from_table("credentials"))
		{
			std::string name;
			std::string username;
			std::string password;
			std::string uri;
			bool name_found{ false };
			for (const database_value& col : row)
			{
				if (col.get_column_name() == "name")
				{
					name = col.str();
					name_found = true;
				}
				else if (col.get_column_name() == "username")
				{
					username = col.str();
				}
				else if (col.get_column_name() == "password")
				{
					password = col.str();
				}
				else if (col.get_column_name() == "uri")
				{
					uri = col.str();
				}
			}
			if (name_found)
			{
				m_credentials.emplace_back(name, username, password, uri);
			}
		}
		m_table_ensured = true;
	}
}