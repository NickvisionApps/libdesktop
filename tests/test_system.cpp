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
	EXPECT_TRUE(proc.input("hello\n"));
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
	EXPECT_TRUE(proc.input_line("hello"));
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
	proc.exited_event().add_handler([&eventFired](const process&, const param_event_args<int>&)
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
