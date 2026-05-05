#include <gtest/gtest.h>
#include <filesystem>
#include <libdesktop.h>

using namespace desktop::events;
using namespace desktop::system;

class System_Test : public ::testing::Test
{
protected:
	power_service m_svc;

	void TearDown() override
	{
		m_svc.allow_suspend();
	}
};

TEST_F(System_Test, Environment_execute)
{
#ifdef _WIN32
	std::string output{ environment::execute("echo hello") };
#else
	std::string output{ environment::execute("/bin/echo hello") };
#endif
	EXPECT_FALSE(output.empty());
}

TEST_F(System_Test, Environment_findDependency)
{
#ifdef _WIN32
	std::filesystem::path dep{ environment::find_dependency("cmd.exe", dependency_search_option::global) };
#else
	std::filesystem::path dep{ environment::find_dependency("sh", dependency_search_option::global) };
#endif
	EXPECT_FALSE(dep.empty());
	EXPECT_TRUE(std::filesystem::exists(dep));
}

TEST_F(System_Test, Environment_getDebuggingInformation)
{
	EXPECT_FALSE(environment::get_debugging_information().empty());
}

TEST_F(System_Test, Environment_getDeploymentMode)
{
	deployment_mode mode{ environment::get_deployment_mode() };
	EXPECT_TRUE(mode == deployment_mode::local || mode == deployment_mode::flatpak || mode == deployment_mode::snap);
}

TEST_F(System_Test, Environment_getExecutableDirectory)
{
	std::filesystem::path dir{ environment::get_executable_directory() };
	EXPECT_FALSE(dir.empty());
	EXPECT_TRUE(dir.is_absolute());
}

TEST_F(System_Test, Environment_getExecutablePath)
{
	std::filesystem::path path{ environment::get_executable_path() };
	EXPECT_FALSE(path.empty());
	EXPECT_TRUE(path.is_absolute());
}

TEST_F(System_Test, Environment_getLocale)
{
	EXPECT_FALSE(environment::get_locale().empty());
}

TEST_F(System_Test, Environment_getPathVariable)
{
	std::vector<std::filesystem::path> path{ environment::get_path_variable() };
	EXPECT_FALSE(path.empty());
}

TEST_F(System_Test, Environment_getVariable)
{
	environment::set_variable("LIBDESKTOP_TEST_GET_VAR", "hello");
	EXPECT_EQ(environment::get_variable("LIBDESKTOP_TEST_GET_VAR"), "hello");
}

TEST_F(System_Test, Environment_hasVariable)
{
	environment::set_variable("LIBDESKTOP_TEST_HAS_VAR", "1");
	EXPECT_TRUE(environment::has_variable("LIBDESKTOP_TEST_HAS_VAR"));
	EXPECT_FALSE(environment::has_variable("LIBDESKTOP_TEST_NONEXISTENT_VAR_XYZ_123"));
}

TEST_F(System_Test, Environment_setVariable)
{
	EXPECT_TRUE(environment::set_variable("LIBDESKTOP_TEST_SET_VAR", "world"));
	EXPECT_EQ(environment::get_variable("LIBDESKTOP_TEST_SET_VAR"), "world");
}

TEST_F(System_Test, Environment_testVariable)
{
	environment::set_variable("LIBDESKTOP_TEST_BOOL_VAR", "true");
	EXPECT_TRUE(environment::test_variable("LIBDESKTOP_TEST_BOOL_VAR"));
	environment::set_variable("LIBDESKTOP_TEST_BOOL_VAR", "0");
	EXPECT_FALSE(environment::test_variable("LIBDESKTOP_TEST_BOOL_VAR"));
}

TEST_F(System_Test, PowerService_allowSuspend)
{
	ASSERT_TRUE(m_svc.prevent_suspend());
	ASSERT_TRUE(m_svc.allow_suspend());
	EXPECT_FALSE(m_svc.is_suspended());
}

TEST_F(System_Test, PowerService_initialState)
{
	EXPECT_FALSE(m_svc.is_suspended());
}

TEST_F(System_Test, PowerService_preventSuspend)
{
	ASSERT_TRUE(m_svc.prevent_suspend());
	EXPECT_TRUE(m_svc.is_suspended());
}

TEST_F(System_Test, Process_getPath)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
	EXPECT_EQ(proc.get_path(), "cmd");
#else
	process proc{ "/bin/echo", { "Hello" } };
	EXPECT_EQ(proc.get_path(), "/bin/echo");
#endif
}

TEST_F(System_Test, Process_getArguments)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
	ASSERT_EQ(proc.get_arguments().size(), 2u);
	EXPECT_EQ(proc.get_arguments()[0], "/c");
	EXPECT_EQ(proc.get_arguments()[1], "echo Hello");
#else
	process proc{ "/bin/echo", { "Hello" } };
	ASSERT_EQ(proc.get_arguments().size(), 1u);
	EXPECT_EQ(proc.get_arguments()[0], "Hello");
#endif
}

TEST_F(System_Test, Process_getWorkingDirectoryDefault)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	EXPECT_TRUE(proc.get_working_directory().empty());
}

TEST_F(System_Test, Process_setWorkingDirectory)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	std::filesystem::path tmpDir{ std::filesystem::temp_directory_path() };
	EXPECT_TRUE(proc.set_working_directory(tmpDir));
	EXPECT_EQ(proc.get_working_directory(), tmpDir);
}

TEST_F(System_Test, Process_getStatusCreated)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	EXPECT_EQ(proc.get_status(), process_status::created);
}

TEST_F(System_Test, Process_start)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello, World!" } };
#else
	process proc{ "/bin/echo", { "Hello, World!" } };
#endif
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
}

TEST_F(System_Test, Process_waitForExit)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello, World!" } };
#else
	process proc{ "/bin/echo", { "Hello, World!" } };
#endif
	ASSERT_TRUE(proc.start());
	EXPECT_EQ(proc.wait_for_exit(), 0);
}

TEST_F(System_Test, Process_getExitCode)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
	EXPECT_EQ(proc.get_exit_code(), 0);
}

TEST_F(System_Test, Process_getStandardOutput)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello, World!" } };
#else
	process proc{ "/bin/echo", { "Hello, World!" } };
#endif
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
	EXPECT_FALSE(proc.get_standard_output().empty());
}

TEST_F(System_Test, Process_getStandardError)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
	EXPECT_TRUE(proc.get_standard_error().empty());
}

TEST_F(System_Test, Process_input)
{
#ifdef _WIN32
	process proc{ "powershell", { "-NoProfile", "-Command", "Write-Host ([Console]::In.ReadLine())" } };
#else
	process proc{ "sh", { "-c", "read line; echo \"$line\"" } };
#endif
	ASSERT_TRUE(proc.start());
	EXPECT_TRUE(proc.write("hello\n"));
	proc.wait_for_exit();
	EXPECT_NE(proc.get_standard_output().find("hello"), std::string::npos);
}

TEST_F(System_Test, Process_inputLine)
{
#ifdef _WIN32
	process proc{ "powershell", { "-NoProfile", "-Command", "Write-Host ([Console]::In.ReadLine())" } };
#else
	process proc{ "sh", { "-c", "read line; echo \"$line\"" } };
#endif
	ASSERT_TRUE(proc.start());
	EXPECT_TRUE(proc.write_line("hello"));
	proc.wait_for_exit();
	EXPECT_NE(proc.get_standard_output().find("hello"), std::string::npos);
}

TEST_F(System_Test, Process_getStatusCompleted)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
	EXPECT_EQ(proc.get_status(), process_status::completed);
}

TEST_F(System_Test, Process_exitedEvent)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello" } };
#else
	process proc{ "/bin/echo", { "Hello" } };
#endif
	bool eventFired{ false };
	proc.get_exited_event().add_handler([&eventFired](const process&, const param_event_args<int>&)
	{
		eventFired = true;
	});
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
	EXPECT_TRUE(eventFired);
}

TEST_F(System_Test, Process_kill)
{
#ifdef _WIN32
	process proc{ "ping", { "127.0.0.1", "-n", "100" } };
#else
	process proc{ "/bin/sleep", { "60" } };
#endif
	ASSERT_TRUE(proc.start());
	EXPECT_TRUE(proc.kill());
}

TEST_F(System_Test, Process_pauseResume)
{
#ifdef _WIN32
	process proc{ "ping", { "127.0.0.1", "-n", "100" } };
#else
	process proc{ "/bin/sleep", { "60" } };
#endif
	ASSERT_TRUE(proc.start());
	EXPECT_TRUE(proc.pause());
	EXPECT_EQ(proc.get_status(), process_status::paused);
	EXPECT_TRUE(proc.resume());
	EXPECT_EQ(proc.get_status(), process_status::running);
	proc.kill();
}
