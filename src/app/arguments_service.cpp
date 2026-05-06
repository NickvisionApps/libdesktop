#include "app/arguments_service.h"
#include <algorithm>
#include <span>

namespace desktop::app
{
	arguments_service::arguments_service(std::span<char*> argv)
	    : m_arguments{ argv.begin(), argv.end() }
	{
	}

	void arguments_service::add(const std::string& argument)
	{
		std::scoped_lock lock{ m_mutex };
		m_arguments.push_back(argument);
	}

	bool arguments_service::contains(std::string_view argument) const
	{
		std::scoped_lock lock{ m_mutex };
		return std::ranges::find(m_arguments, argument) != m_arguments.end();
	}

	const std::vector<std::string>& arguments_service::get_all() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_arguments;
	}

	size_t arguments_service::get_count() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_arguments.size();
	}

	std::optional<std::string> arguments_service::get_next(std::string_view argument) const
	{
		std::scoped_lock lock{ m_mutex };
		auto it = std::ranges::find(m_arguments, argument);
		if (it != m_arguments.end())
		{
			it++;
			if (it != m_arguments.end())
			{
				return *it;
			}
		}
		return std::nullopt;
	}
}