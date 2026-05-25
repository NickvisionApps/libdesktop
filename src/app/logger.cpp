#include "app/logger.h"
#include <format>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

namespace desktop::app
{
	static std::string_view log_type_to_color(log_type type)
	{
		switch (type)
		{
		case log_type::debug:
			return "\033[90m";
		case log_type::info:
			return "\033[32m";
		case log_type::warn:
			return "\033[33m";
		case log_type::error:
			return "\033[31m";
		case log_type::critical:
			return "\033[35m";
		default:
			return "\033[0m";
		}
	}

	static std::string_view log_type_to_string(log_type type)
	{
		switch (type)
		{
		case log_type::debug:
			return "DEBUG";
		case log_type::info:
			return "INFO";
		case log_type::warn:
			return "WARN";
		case log_type::error:
			return "ERROR";
		case log_type::critical:
			return "CRITICAL";
		default:
			return "UNKNOWN";
		}
	}

	logger::logger(log_type minimum, const std::filesystem::path& log_path)
	    : m_minimum{ minimum }
	{
#ifdef _WIN32
		HANDLE h_out{ GetStdHandle(STD_OUTPUT_HANDLE) };
		HANDLE h_err{ GetStdHandle(STD_ERROR_HANDLE) };
		DWORD mode{};
		if (GetConsoleMode(h_out, &mode) != 0)
		{
			SetConsoleMode(h_out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
		if (GetConsoleMode(h_err, &mode) != 0)
		{
			SetConsoleMode(h_err, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
#endif
		if (!log_path.empty())
		{
			m_file = std::ofstream{ log_path, std::ios::trunc };
		}
	}

	logger::~logger()
	{
		if (m_file.is_open())
		{
			try
			{
				m_file.flush();
			}
			catch (...)
			{
			}
		}
	}

	bool logger::critical(std::string_view message, const std::source_location& location)
	{
		return log(log_type::critical, message, location);
	}

	bool logger::debug(std::string_view message, const std::source_location& location)
	{
		return log(log_type::debug, message, location);
	}

	bool logger::error(std::string_view message, const std::source_location& location)
	{
		return log(log_type::error, message, location);
	}

	bool logger::info(std::string_view message, const std::source_location& location)
	{
		return log(log_type::info, message, location);
	}

	bool logger::warn(std::string_view message, const std::source_location& location)
	{
		return log(log_type::warn, message, location);
	}

	bool logger::log(log_type type, std::string_view message, const std::source_location& location)
	{
		constexpr std::string_view reset{ "\033[0m" };
		std::scoped_lock lock{ m_mutex };
		if (type < m_minimum)
		{
			return false;
		}
		bool is_severe{ type == log_type::error || type == log_type::critical };
		std::string msg{ std::format("[{}] {} ({}:{})", log_type_to_string(type), message, location.file_name(), location.line()) };
		if (is_severe)
		{
			std::cerr << log_type_to_color(type) << msg << reset << '\n';
		}
		else
		{
			std::cout << log_type_to_color(type) << msg << reset << '\n';
		}
		if (m_file.is_open())
		{
			m_file << msg << '\n';
			if (is_severe)
			{
				m_file.flush();
			}
		}
		return true;
	}
}