#include "app/arguments_service.h"
#include <algorithm>

namespace desktop::app
{
	arguments_service::arguments_service(int argc, char* argv[])
		: m_arguments{ argv, argv + argc }
	{

	}

	void arguments_service::add(const std::string& argument)
	{
		m_arguments.push_back(argument);
	}

	bool arguments_service::contains(std::string_view argument) const
	{
		return std::find(m_arguments.begin(), m_arguments.end(), argument) != m_arguments.end();
	}

	const std::vector<std::string>& arguments_service::get_all() const
	{
		return m_arguments;
	}

	size_t arguments_service::get_count() const
	{
		return m_arguments.size();
	}

	std::optional<std::string> arguments_service::get_next(std::string_view argument) const
	{
		auto it = std::find(m_arguments.begin(), m_arguments.end(), argument);
		if (it != m_arguments.end() && ++it != m_arguments.end())
		{
			return *it;
		}
		return std::nullopt;
	}
}