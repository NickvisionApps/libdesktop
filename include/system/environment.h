#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "dependency_search_option.h"
#include "deployment_mode.h"

namespace desktop::system::environment
{
	std::string execute(const std::string& command);
	const std::filesystem::path& find_dependency(std::string_view name, dependency_search_option option);
	std::string get_debugging_information();
	deployment_mode get_deployment_mode();
	std::filesystem::path get_executable_directory();
	std::filesystem::path get_executable_path();
	std::string get_locale();
	std::vector<std::filesystem::path> get_path_variable();
	bool clear_variable(const std::string& name);
	std::string get_variable(const std::string& name);
	bool has_variable(const std::string& name);
	bool set_variable(const std::string& name, const std::string& value);
	bool test_variable(const std::string& name);
	std::string get_last_system_error_message();
}
