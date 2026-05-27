#pragma once

#include <string>

namespace desktop::system
{
	class process_result
	{
	public:
		process_result();
		process_result(std::string output, std::string error, int exit_code);
		~process_result() = default;
		process_result(const process_result&) = default;
		process_result(process_result&&) = default;
		const std::string& get_output() const;
		const std::string& get_error() const;
		int get_exit_code() const;
		process_result& operator=(const process_result&) = default;
		process_result& operator=(process_result&&) = default;

	private:
		std::string m_output;
		std::string m_error;
		int m_exit_code;
	};
}