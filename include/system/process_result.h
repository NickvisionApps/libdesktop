#pragma once

#include <string_view>

namespace desktop::system
{
	class process_result
	{
	public:
		process_result();
		process_result(std::string_view output, std::string_view error, int exit_code);
		~process_result() = default;
		process_result(const process_result&) = default;
		process_result(process_result&&) = default;
		std::string_view get_output() const;
		std::string_view get_error() const;
		int get_exit_code() const;
		process_result& operator=(const process_result&) = default;
		process_result& operator=(process_result&&) = default;

	private:
		std::string_view m_output;
		std::string_view m_error;
		int m_exit_code;
	};
}