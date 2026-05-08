#include "secrets/credential.h"

namespace desktop::secrets
{
	credential::credential(std::string name, std::string username, std::string password, std::string url)
	    : m_name{ std::move(name) },
	      m_username{ std::move(username) },
	      m_password{ std::move(password) },
	      m_url{ std::move(url) }
	{
	}

	const std::string& credential::get_name() const
	{
		return m_name;
	}

	void credential::set_name(const std::string& name)
	{
		m_name = name;
	}

	const std::string& credential::get_username() const
	{
		return m_username;
	}

	void credential::set_username(const std::string& username)
	{
		m_username = username;
	}

	const std::string& credential::get_password() const
	{
		return m_password;
	}

	void credential::set_password(const std::string& password)
	{
		m_password = password;
	}

	const std::string& credential::get_url() const
	{
		return m_url;
	}

	void credential::set_url(const std::string& url)
	{
		m_url = url;
	}

	std::strong_ordering credential::operator<=>(const credential& other) const
	{
		return m_name <=> other.m_name;
	}

	bool credential::operator==(const credential& other) const
	{
		return m_name == other.m_name;
	}

	bool credential::operator!=(const credential& other) const
	{
		return !(operator==(other));
	}
}