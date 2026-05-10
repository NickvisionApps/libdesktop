#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>
#include "log_type.h"

namespace desktop::app
{
	class logger
	{
	public:
		logger(const std::filesystem::path& log_path = {});
		~logger();
		logger(const logger&) = delete;
		logger(logger&&) = delete;
		void critical(std::string_view message, std::string_view file, unsigned int line);
		void debug(std::string_view message, std::string_view file, unsigned int line);
		void error(std::string_view message, std::string_view file, unsigned int line);
		void info(std::string_view message, std::string_view file, unsigned int line);
		void warn(std::string_view message, std::string_view file, unsigned int line);
		void log(log_type type, std::string_view message, std::string_view file, unsigned int line);
		logger& operator=(const logger&) = delete;
		logger& operator=(logger&&) = delete;

	private:
		mutable std::mutex m_mutex;
		std::ofstream m_file;
	};
}