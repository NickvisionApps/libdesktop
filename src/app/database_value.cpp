#include "app/database_value.h"

namespace desktop::app
{
	database_value::database_value(const std::string& name, bool v)
	    : m_column_name{ name },
	      m_value{ static_cast<int64_t>(v ? 1 : 0) }
	{
	}

	database_value::database_value(const std::string& name, int v)
	    : m_column_name{ name },
	      m_value{ static_cast<int64_t>(v) }
	{
	}

	database_value::database_value(const std::string& name, int64_t v)
	    : m_column_name{ name },
	      m_value{ v }
	{
	}

	database_value::database_value(const std::string& name, double v)
	    : m_column_name{ name },
	      m_value{ v }
	{
	}

	database_value::database_value(const std::string& name, float v)
	    : m_column_name{ name },
	      m_value{ static_cast<double>(v) }
	{
	}

	database_value::database_value(const std::string& name, const std::string& v)
	    : m_column_name{ name },
	      m_value{ v }
	{
	}

	database_value::database_value(const std::string& name, std::string_view v)
	    : m_column_name{ name },
	      m_value{ std::string{ v } }
	{
	}

	database_value::database_value(const std::string& name, const char* v)
	    : m_column_name{ name },
	      m_value{ std::string{ v ? v : "" } }
	{
	}

	database_value::database_value(const std::string& name, std::nullptr_t)
	    : m_column_name{ name },
	      m_value{ nullptr }
	{
	}

	const std::variant<int64_t, double, std::string, std::nullptr_t>& database_value::get_value() const
	{
		return m_value;
	}

	const std::string& database_value::get_column_name() const
	{
		return m_column_name;
	}

	std::string database_value::str() const
	{
		return std::visit([](auto&& v) -> std::string
		{
			using T = std::decay_t<decltype(v)>;
			if constexpr (std::is_same_v<T, std::string>)
			{
				return v;
			}
			else if constexpr (std::is_same_v<T, int64_t>)
			{
				return std::to_string(v);
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				return std::to_string(v);
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				return v ? "true" : "false";
			}
			else
			{
				return {};
			}
		}, m_value);
	}
}
