#include "secrets/secret.h"

namespace desktop::secrets
{
	secret::secret(const std::string& name, const std::string& value)
		: m_name{ name },
		m_value{ value }
	{

	}

	const std::string& secret::name() const
	{
		return m_name;
	}

	const std::string& secret::value() const
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