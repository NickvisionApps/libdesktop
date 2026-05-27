#include "system/process_result.h"

namespace desktop::system
{
	process_result::process_result()
	    : m_exit_code{ -1 }
	{
	}

	process_result::process_result(std::string output, std::string error, int exit_code)
	    : m_output{ std::move(output) },
	      m_error{ std::move(error) },
	      m_exit_code{ exit_code }
	{
	}

	const std::string& process_result::get_output() const
	{
		return m_output;
	}

	const std::string& process_result::get_error() const
	{
		return m_error;
	}

	int process_result::get_exit_code() const
	{
		return m_exit_code;
	}
}