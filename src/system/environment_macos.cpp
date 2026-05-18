#include "system/environment.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <mach-o/dyld.h>
#include <sstream>
#include <sys/syslimits.h>
#include <unordered_map>
#include "filesystem/user_directories.h"
#include "helpers/string_manip.h"
#include "system/process.h"

using namespace desktop::filesystem;
using namespace desktop::helpers;

static bool search_in(const std::filesystem::path& dep, std::filesystem::path& result, const std::filesystem::path& dir)
{
	std::filesystem::path candidate{ dir / dep };
	if (!std::filesystem::exists(candidate))
	{
		return false;
	}
	result = candidate;
	return true;
}

namespace desktop::system
{
	process_result environment::execute(const std::string& command)
	{
		if (command.empty())
		{
			return {};
		}
		std::vector<std::string> args{ string_manip::split(command, ' ', false) };
		std::string cmd{ args[0] };
		args.erase(args.begin());
		process proc{ cmd, args };
		if (proc.start())
		{
			proc.wait_for_exit();
			return proc.get_result();
		}
		return {};
	}

	const std::filesystem::path& environment::find_dependency(std::string_view name, dependency_search_option option)
	{
		static std::unordered_map<std::string, std::filesystem::path> dependencies;
		std::filesystem::path dep{ name };
		std::string key{ dep.string() + "|" + std::to_string(static_cast<int>(option)) };
		auto it{ dependencies.find(key) };
		if (it != dependencies.end() && std::filesystem::exists(it->second))
		{
			return it->second;
		}
		dependencies[key] = std::filesystem::path();
		std::filesystem::path& result{ dependencies[key] };
		if (option == dependency_search_option::global || option == dependency_search_option::app)
		{
			search_in(dep, result, get_executable_directory());
		}
		if (result.empty() && (option == dependency_search_option::global || option == dependency_search_option::system))
		{
			for (const std::filesystem::path& dir : get_path_variable())
			{
				if (search_in(dep, result, dir))
				{
					break;
				}
			}
		}
		if (result.empty() && option == dependency_search_option::local)
		{
			search_in(dep, result, user_directories::get_local_data());
		}
		return result;
	}

	std::string environment::get_debugging_information()
	{
		std::ostringstream builder;
		builder << "Operating System: macOS\n";
		builder << "Deployment Mode: Local\n";
		builder << "Locale: " << get_locale() << "\n";
		builder << "Running From: " << get_executable_directory().string() << "\n";
		return builder.str();
	}

	deployment_mode environment::get_deployment_mode()
	{
		return deployment_mode::local;
	}

	std::filesystem::path environment::get_executable_directory()
	{
		return get_executable_path().parent_path();
	}

	std::filesystem::path environment::get_executable_path()
	{
		char path[PATH_MAX + 1];
		uint32_t size{ sizeof(path) };
		if (_NSGetExecutablePath(path, &size) == 0)
		{
			return std::filesystem::canonical(path);
		}
		return {};
	}

	std::string environment::get_locale()
	{
		try
		{
			return std::locale("").name();
		}
		catch (...)
		{
			return {};
		}
	}

	std::vector<std::filesystem::path> environment::get_path_variable()
	{
		std::string env{ get_variable("PATH") };
		if (!env.empty())
		{
			return string_manip::split<std::filesystem::path>(env, ':');
		}
		return {};
	}

	bool environment::clear_variable(const std::string& name)
	{
		return unsetenv(name.c_str()) == 0;
	}

	std::string environment::get_variable(const std::string& name)
	{
		char* res{ std::getenv(name.c_str()) };
		return res != nullptr ? std::string{ res } : std::string{};
	}

	bool environment::has_variable(const std::string& name)
	{
		return std::getenv(name.c_str()) != nullptr;
	}

	bool environment::set_variable(const std::string& name, const std::string& value)
	{
		return setenv(name.c_str(), value.c_str(), true) == 0;
	}

	bool environment::test_variable(const std::string& name)
	{
		std::string value{ string_manip::lower(get_variable(name)) };
		if (value.empty())
		{
			return false;
		}
		return value == "true" || value == "1" || value == "yes" || value == "on" || value == "t" || value == "y" || value == "enable" || value == "enabled";
	}

	std::string environment::get_last_system_error_message()
	{
		return { std::strerror(errno) };
	}
}
