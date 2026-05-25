#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <source_location>
#include <string_view>
#include "log_type.h"

namespace desktop::app
{
	class logger
	{
	public:
		logger(log_type minimum, const std::filesystem::path& log_path = {});
		~logger();
		logger(const logger&) = delete;
		logger(logger&&) = delete;
		bool critical(std::string_view message, const std::source_location& location = std::source_location::current());
		bool debug(std::string_view message, const std::source_location& location = std::source_location::current());
		bool error(std::string_view message, const std::source_location& location = std::source_location::current());
		bool info(std::string_view message, const std::source_location& location = std::source_location::current());
		bool warn(std::string_view message, const std::source_location& location = std::source_location::current());
		bool log(log_type type, std::string_view message, const std::source_location& location = std::source_location::current());
		logger& operator=(const logger&) = delete;
		logger& operator=(logger&&) = delete;

	private:
		mutable std::mutex m_mutex;
		log_type m_minimum;
		std::ofstream m_file;
	};
}