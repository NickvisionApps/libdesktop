#include "hosting/host_options.h"
#include <span>

namespace desktop::hosting
{
	host_options::host_options(std::shared_ptr<app::app_info> app_info, std::span<char*> argv)
	    : m_app_info{ std::move(app_info) },
	      m_argv{ argv },
	      m_single_instance{ false }
	{
	}

	const std::shared_ptr<app::app_info>& host_options::get_app_info() const
	{
		return m_app_info;
	}

	std::span<char*> host_options::get_argv() const
	{
		return m_argv;
	}

	bool host_options::is_single_instance() const
	{
		return m_single_instance;
	}

	void host_options::set_single_instance(bool single)
	{
		m_single_instance = single;
	}
}