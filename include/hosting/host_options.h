#pragma once

#include <filesystem>

namespace desktop::hosting
{
	class host_options
	{
	public:
		host_options(int argc, char* argv[]);
		int get_argc() const;
		char** get_argv() const;
		const std::filesystem::path& get_log_path() const;
		void set_log_path(const std::filesystem::path& log_path);

	private:
		int m_argc;
		char** m_argv;
		std::filesystem::path m_log_path;
	};
}