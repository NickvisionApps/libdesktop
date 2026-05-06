#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace desktop::database
{
	class database_value
	{
	public:
		database_value(std::string name, bool v);
		database_value(std::string name, int v);
		database_value(std::string name, int64_t v);
		database_value(std::string name, double v);
		database_value(std::string name, float v);
		database_value(std::string name, std::string v);
		database_value(std::string name, std::string_view v);
		database_value(std::string name, const char* v);
		database_value(std::string name, std::nullptr_t);
		template <typename T>
		    requires std::is_enum_v<T>
		database_value(std::string name, T v)
		    : m_column_name{ std::move(name) },
		      m_value{ static_cast<int64_t>(v) }
		{
		}
		~database_value() = default;
		database_value(const database_value&) = default;
		database_value(database_value&&) noexcept = default;
		const std::variant<int64_t, double, std::string, std::nullptr_t>& get_value() const;
		const std::string& get_column_name() const;
		std::string str() const;
		database_value& operator=(const database_value&) = default;
		database_value& operator=(database_value&&) noexcept = default;

	private:
		std::string m_column_name;
		std::variant<int64_t, double, std::string, std::nullptr_t> m_value;
	};
}
