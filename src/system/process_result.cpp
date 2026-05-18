#include "system/process_result.h"

namespace desktop::system
{
	process_result::process_result(std::string_view output, std::string_view error, int exit_code)
	    : m_output{ output },
	      m_error{ error },
	      m_exit_code{ exit_code }
	{
	}

	std::string_view process_result::get_output() const
	{
		return m_output;
	}

	std::string_view process_result::get_error() const
	{
		return m_error;
	}

	int process_result::get_exit_code() const
	{
		return m_exit_code;
	}
}