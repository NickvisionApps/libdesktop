#include "hosting/host_options.h"

namespace desktop::hosting
{
	host_options::host_options(int argc, char* argv[])
		: m_argc{ argc },
		m_argv{ argv }
	{

	}

	int host_options::get_argc() const
	{
		return m_argc;
	}

	char** host_options::get_argv() const
	{
		return m_argv;
	}

	const std::shared_ptr<app::app_info>& host_options::get_app_info() const
	{
		return m_app_info;
	}

	void host_options::set_app_info(const std::shared_ptr<app::app_info>& app_info)
	{
		m_app_info = app_info;
	}

	const std::filesystem::path& host_options::get_log_path() const
	{
		return m_log_path;
	}

	void host_options::set_log_path(const std::filesystem::path& log_path)
	{
		m_log_path = log_path;
	}
}