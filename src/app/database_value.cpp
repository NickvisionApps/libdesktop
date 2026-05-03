#include "app/database_value.h"

namespace desktop::app
{
	database_value::database_value(int v)
		: m_value{ static_cast<int64_t>(v) }
	{

	}

	database_value::database_value(int64_t v)
		: m_value{ v }
	{

	}

	database_value::database_value(double v)
		: m_value{ v }
	{

	}

	database_value::database_value(float v)
		: m_value{ static_cast<double>(v) }
	{

	}

	database_value::database_value(const std::string& v)
		: m_value{ v }
	{

	}

	database_value::database_value(std::string_view v)
		: m_value{ std::string{ v } }
	{

	}

	database_value::database_value(const char* v)
		: m_value{ std::string{ v ? v : "" } }
	{

	}

	database_value::database_value(std::nullptr_t)
		: m_value{ nullptr }
	{

	}

	const std::variant<int64_t, double, std::string, std::nullptr_t>& database_value::value() const
	{
		return m_value;
	}
}
