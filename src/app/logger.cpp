#include <format>
#include <iostream>
#include "app/logger.h"
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
			return "\033[1;31m";
		default:
			return "\033[0m";
		}
	}

	static const std::string& log_type_to_string(log_type type)
	{
		static std::string debug{ "DEBUG" };
		static std::string info{ "INFO" };
		static std::string warn{ "WARN" };
		static std::string error{ "ERROR" };
		static std::string critical{ "CRITICAL" };
		static std::string unknown{ "UNKNOWN" };
		switch (type)
		{
		case log_type::debug:
			return debug;
		case log_type::info:
			return info;
		case log_type::warn:
			return warn;
		case log_type::error:
			return error;
		case log_type::critical:
			return critical;
		default:
			return unknown;
		}
	}

	logger::logger(const std::filesystem::path& log_path)
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

	void logger::critical(std::string_view message, std::string_view file, unsigned int line)
	{
		log(log_type::critical, message, file, line);
	}

	void logger::debug(std::string_view message, std::string_view file, unsigned int line)
	{
		log(log_type::debug, message, file, line);
	}

	void logger::error(std::string_view message, std::string_view file, unsigned int line)
	{
		log(log_type::error, message, file, line);
	}

	void logger::info(std::string_view message, std::string_view file, unsigned int line)
	{
		log(log_type::info, message, file, line);
	}

	void logger::warn(std::string_view message, std::string_view file, unsigned int line)
	{
		log(log_type::warn, message, file, line);
	}

	void logger::log(log_type type, std::string_view message, std::string_view file, unsigned int line)
	{
		constexpr std::string_view reset{ "\033[0m" };
		std::scoped_lock lock{ m_mutex };
		bool is_severe{ type == log_type::error || type == log_type::critical };
		std::string file_name{ std::filesystem::path(file).filename().string() };
		std::string msg{ std::format("[{}] {} ({}:{})", log_type_to_string(type), message, file_name, line) };
		if (is_severe)
		{
			std::cerr << log_type_to_color(type) << msg << reset << '\n';
		}
		else
		{
#ifdef NDEBUG
			if (type != log_type::debug)
			{
#endif
				std::cout << log_type_to_color(type) << msg << reset << '\n';
#ifdef NDEBUG
			}
#endif
		}
		if (m_file.is_open())
		{
			m_file << msg << '\n';
			if (is_severe)
			{
				m_file.flush();
			}
		}
	}
}