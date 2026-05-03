#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

namespace desktop::app
{
	class database_value
	{
	public:
		~database_value() = default;
		database_value(const database_value&) = default;
		database_value(database_value&&) noexcept = default;
		database_value(int v);
		database_value(int64_t v);
		database_value(double v);
		database_value(float v);
		database_value(const std::string& v);
		database_value(std::string_view v);
		database_value(const char* v);
		database_value(std::nullptr_t);
		const std::variant<int64_t, double, std::string, std::nullptr_t>& value() const;
		database_value& operator=(const database_value&) = default;
		database_value& operator=(database_value&&) noexcept = default;

	private:
		std::variant<int64_t, double, std::string, std::nullptr_t> m_value;
	};
}
