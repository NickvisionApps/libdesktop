#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <string>
#include <vector>

using namespace desktop::system;

#ifdef _WIN32
static const std::string echo_command{ "cmd /c echo hello" };
static const std::string invalid_command{ "C:\\does_not_exist\\missing.exe" };
static constexpr char path_separator{ ';' };
static const std::string known_dependency{ "cmd.exe" };
#else
static const std::string echo_command{ "echo hello" };
static const std::string invalid_command{ "/does/not/exist" };
static constexpr char path_separator{ ':' };
static const std::string known_dependency{ "sh" };
#endif

TEST(Environment, Execute)
{
	process_result result{ environment::execute(echo_command) };
	ASSERT_EQ(result.get_exit_code(), 0);
	ASSERT_NE(result.get_output().find("hello"), std::string::npos);
}

TEST(Environment, ExecuteEmpty)
{
	process_result result{ environment::execute("") };
	ASSERT_EQ(result.get_exit_code(), -1);
	ASSERT_TRUE(result.get_output().empty());
	ASSERT_TRUE(result.get_error().empty());
}

TEST(Environment, ExecuteInvalid)
{
	process_result result{ environment::execute(invalid_command) };
	ASSERT_NE(result.get_exit_code(), 0);
}

TEST(Environment, GetExecutablePath)
{
	std::filesystem::path exe{ environment::get_executable_path() };
	ASSERT_FALSE(exe.empty());
	ASSERT_TRUE(std::filesystem::exists(exe));
}

TEST(Environment, GetExecutableDirectory)
{
	std::filesystem::path dir{ environment::get_executable_directory() };
	ASSERT_FALSE(dir.empty());
	ASSERT_TRUE(std::filesystem::exists(dir));
	ASSERT_TRUE(std::filesystem::is_directory(dir));
}

TEST(Environment, ExecutableDirectoryMatchesPathParent)
{
	ASSERT_EQ(environment::get_executable_directory(), environment::get_executable_path().parent_path());
}

TEST(Environment, GetLocale)
{
	std::string locale{ environment::get_locale() };
	SUCCEED();
}

TEST(Environment, GetPathVariable)
{
	std::vector<std::filesystem::path> paths{ environment::get_path_variable() };
	ASSERT_FALSE(paths.empty());
}

TEST(Environment, PathVariableContainsValidEntries)
{
	std::vector<std::filesystem::path> paths{ environment::get_path_variable() };
	bool found_non_empty{ false };
	for (const auto& path : paths)
	{
		if (!path.empty())
		{
			found_non_empty = true;
			break;
		}
	}
	ASSERT_TRUE(found_non_empty);
}

TEST(Environment, SetVariable)
{
	ASSERT_TRUE(environment::set_variable("LIBDESKTOP_TEST_VAR", "123"));
	ASSERT_EQ(environment::get_variable("LIBDESKTOP_TEST_VAR"), "123");
}

TEST(Environment, HasVariable)
{
	environment::set_variable("LIBDESKTOP_HAS_VAR", "1");
	ASSERT_TRUE(environment::has_variable("LIBDESKTOP_HAS_VAR"));
}

TEST(Environment, ClearVariable)
{
	environment::set_variable("LIBDESKTOP_CLEAR_VAR", "1");
	ASSERT_TRUE(environment::clear_variable("LIBDESKTOP_CLEAR_VAR"));
	ASSERT_FALSE(environment::has_variable("LIBDESKTOP_CLEAR_VAR"));
}

TEST(Environment, GetVariableMissing)
{
	ASSERT_EQ(environment::get_variable("LIBDESKTOP_VARIABLE_DOES_NOT_EXIST"), "");
}

TEST(Environment, HasVariableMissing)
{
	ASSERT_FALSE(environment::has_variable("LIBDESKTOP_VARIABLE_DOES_NOT_EXIST"));
}

TEST(Environment, TestVariableTrue)
{
	environment::set_variable("LIBDESKTOP_BOOL_VAR", "true");
	ASSERT_TRUE(environment::test_variable("LIBDESKTOP_BOOL_VAR"));
}

TEST(Environment, TestVariableFalse)
{
	environment::set_variable("LIBDESKTOP_BOOL_VAR", "false");
	ASSERT_FALSE(environment::test_variable("LIBDESKTOP_BOOL_VAR"));
}

TEST(Environment, TestVariableNumericTrue)
{
	environment::set_variable("LIBDESKTOP_BOOL_VAR", "1");
	ASSERT_TRUE(environment::test_variable("LIBDESKTOP_BOOL_VAR"));
}

TEST(Environment, TestVariableNumericFalse)
{
	environment::set_variable("LIBDESKTOP_BOOL_VAR", "0");
	ASSERT_FALSE(environment::test_variable("LIBDESKTOP_BOOL_VAR"));
}

TEST(Environment, TestVariableCaseInsensitive)
{
	environment::set_variable("LIBDESKTOP_BOOL_VAR", "YeS");
	ASSERT_TRUE(environment::test_variable("LIBDESKTOP_BOOL_VAR"));
}

TEST(Environment, TestVariableMissing)
{
	environment::clear_variable("LIBDESKTOP_BOOL_VAR");
	ASSERT_FALSE(environment::test_variable("LIBDESKTOP_BOOL_VAR"));
}

TEST(Environment, FindDependencySystem)
{
	const std::filesystem::path& dep{ environment::find_dependency(known_dependency, dependency_search_option::system) };
	ASSERT_FALSE(dep.empty());
	ASSERT_TRUE(std::filesystem::exists(dep));
}

TEST(Environment, FindDependencyMissing)
{
	const std::filesystem::path& dep{ environment::find_dependency("definitely_missing_binary_12345", dependency_search_option::system) };
	ASSERT_TRUE(dep.empty());
}

TEST(Environment, FindDependencyCachesResult)
{
	const std::filesystem::path& first{ environment::find_dependency(known_dependency, dependency_search_option::system) };
	const std::filesystem::path& second{ environment::find_dependency(known_dependency, dependency_search_option::system) };
	ASSERT_EQ(first, second);
}

TEST(Environment, FindDependencyApp)
{
	std::filesystem::path exe_dir{ environment::get_executable_directory() };
#ifdef _WIN32
	std::filesystem::path temp_file{ exe_dir / "libdesktop_test_dep.exe" };
#else
	std::filesystem::path temp_file{ exe_dir / "libdesktop_test_dep" };
#endif
	{
		std::ofstream stream{ temp_file.string() };
		stream << "test";
	}
	const std::filesystem::path& dep{ environment::find_dependency(temp_file.filename().string(), dependency_search_option::app) };
	ASSERT_EQ(dep, temp_file);
	std::filesystem::remove(temp_file);
}

TEST(Environment, GetDebuggingInformation)
{
	std::string info{ environment::get_debugging_information() };
	ASSERT_FALSE(info.empty());
	ASSERT_NE(info.find("Locale"), std::string::npos);
	ASSERT_NE(info.find("Running From"), std::string::npos);
}

TEST(Environment, GetDeploymentMode)
{
	deployment_mode mode{ environment::get_deployment_mode() };
	switch (mode)
	{
	case deployment_mode::local:
	case deployment_mode::flatpak:
	case deployment_mode::snap:
	case deployment_mode::wsl:
		SUCCEED();
		break;
	default:
		FAIL();
	}
}

TEST(Environment, GetLastSystemErrorMessage)
{
	std::ifstream file{ "/definitely/invalid/file/path" };
	std::string error{ environment::get_last_system_error_message() };
	ASSERT_FALSE(error.empty());
}

TEST(Environment, PathVariableSeparatorHandling)
{
	std::string original{ environment::get_variable("PATH") };
	ASSERT_NE(original.find(path_separator), std::string::npos);
	std::vector<std::filesystem::path> parsed{ environment::get_path_variable() };
	ASSERT_FALSE(parsed.empty());
}

TEST(Environment, SetVariableOverwrite)
{
	ASSERT_TRUE(environment::set_variable("LIBDESKTOP_OVERWRITE_VAR", "first"));
	ASSERT_EQ(environment::get_variable("LIBDESKTOP_OVERWRITE_VAR"), "first");
	ASSERT_TRUE(environment::set_variable("LIBDESKTOP_OVERWRITE_VAR", "second"));
	ASSERT_EQ(environment::get_variable("LIBDESKTOP_OVERWRITE_VAR"), "second");
}