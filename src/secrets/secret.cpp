#include "secrets/secret.h"

namespace desktop::secrets
{
	secret::secret(std::string name, std::string value)
	    : m_name{ std::move(name) },
	      m_value{ std::move(value) }
	{
	}

	const std::string& secret::get_name() const
	{
		return m_name;
	}

	const std::string& secret::get_value() const
	{
		return m_value;
	}

	bool secret::empty() const
	{
		return m_name.empty() && m_value.empty();
	}

	secret::operator bool() const
	{
		return !empty();
	}
}