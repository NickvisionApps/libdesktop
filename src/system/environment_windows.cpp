#include "system/environment.h"
#include <windows.h>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include "filesystem/user_directories.h"
#include "helpers/string_manip.h"
#include "system/process.h"

using namespace desktop::filesystem;
using namespace desktop::helpers;

static bool search_in(const std::filesystem::path& dep, std::filesystem::path& result, const std::filesystem::path& dir)
{
	if (dir.string().find("AppData\\Local\\Microsoft\\WindowsApps") != std::string::npos)
	{
		return false;
	}
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
	static std::unordered_map<std::string, std::filesystem::path> dependencies;

	std::string environment::execute(const std::string& command)
	{
		if (command.empty())
		{
			return {};
		}
		process proc{ find_dependency("cmd.exe", dependency_search_option::global), { "/c", command } };
		if (proc.start())
		{
			proc.wait_for_exit();
			return proc.get_standard_output();
		}
		return {};
	}

	const std::filesystem::path& environment::find_dependency(std::string_view name, dependency_search_option option)
	{
		std::filesystem::path dep{ name };
		if (!dep.has_extension())
		{
			dep += ".exe";
		}
		std::string key{ dep.string() + "|" + std::to_string(static_cast<int>(option)) };
		std::unordered_map<std::string, std::filesystem::path>::const_iterator it{ dependencies.find(key) };
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
		builder << "Operating System: Windows\n";
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
		wchar_t path[MAX_PATH];
		DWORD len{ GetModuleFileNameW(nullptr, path, MAX_PATH) };
		if (len > 0)
		{
			return std::filesystem::path{ std::wstring(path, len) };
		}
		return {};
	}

	std::string environment::get_locale()
	{
		LCID lcid{ GetThreadLocale() };
		wchar_t name[LOCALE_NAME_MAX_LENGTH];
		if (LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0) > 0)
		{
			return string_manip::str(name);
		}
		return {};
	}

	std::vector<std::filesystem::path> environment::get_path_variable()
	{
		std::string env{ get_variable("PATH") };
		if (!env.empty())
		{
			return string_manip::split<std::filesystem::path>(env, ';');
		}
		return {};
	}

	std::string environment::get_variable(std::string_view name)
	{
		char* res{ std::getenv(name.data()) };
		return res ? std::string{ res } : std::string{};
	}

	bool environment::has_variable(std::string_view name)
	{
		return std::getenv(name.data());
	}

	bool environment::set_variable(std::string_view name, std::string_view value)
	{
		return _putenv_s(name.data(), value.data()) == 0;
	}

	bool environment::test_variable(std::string_view name)
	{
		std::string value{ string_manip::lower(get_variable(name)) };
		if (value.empty())
		{
			return false;
		}
		return value == "true" || value == "1" || value == "yes" || value == "on" || value == "t" || value == "y" || value == "enable" || value == "enabled";
	}
}
