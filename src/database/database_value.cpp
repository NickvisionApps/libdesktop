#include "database/database_value.h"

namespace desktop::database
{
	database_value::database_value(std::string name, bool v)
	    : m_column_name{ std::move(name) },
	      m_value{ static_cast<int64_t>(v ? 1 : 0) }
	{
	}

	database_value::database_value(std::string name, int v)
	    : m_column_name{ std::move(name) },
	      m_value{ static_cast<int64_t>(v) }
	{
	}

	database_value::database_value(std::string name, int64_t v)
	    : m_column_name{ std::move(name) },
	      m_value{ v }
	{
	}

	database_value::database_value(std::string name, double v)
	    : m_column_name{ std::move(name) },
	      m_value{ v }
	{
	}

	database_value::database_value(std::string name, float v)
	    : m_column_name{ std::move(name) },
	      m_value{ static_cast<double>(v) }
	{
	}

	database_value::database_value(std::string name, std::string v)
	    : m_column_name{ std::move(name) },
	      m_value{ std::move(v) }
	{
	}

	database_value::database_value(std::string name, std::string_view v)
	    : m_column_name{ std::move(name) },
	      m_value{ std::string{ v } }
	{
	}

	database_value::database_value(std::string name, const char* v)
	    : m_column_name{ std::move(name) },
	      m_value{ std::string{ v != nullptr ? v : "" } }
	{
	}

	database_value::database_value(std::string name, std::nullptr_t)
	    : m_column_name{ std::move(name) },
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
			using value_t = std::decay_t<decltype(v)>;
			if constexpr (std::is_same_v<value_t, std::string>)
			{
				return v;
			}
			else if constexpr (std::is_same_v<value_t, int64_t>)
			{ // NOLINT(bugprone-branch-clone)
				return std::to_string(v);
			}
			else if constexpr (std::is_same_v<value_t, double>)
			{
				return std::to_string(v);
			}
			else if constexpr (std::is_same_v<value_t, bool>)
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
