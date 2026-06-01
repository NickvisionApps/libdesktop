#include <chrono>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <string>
#include <thread>

using namespace desktop::system;
using namespace desktop::events;

#ifdef _WIN32
static const std::string echo_exe{ "cmd" };
static const std::vector<std::string> echo_hello{ "/c", "echo hello" };
static const std::string sleep_exe{ "ping" };
static const std::vector<std::string> sleep_args{ "-n", "11", "127.0.0.1" };
static const std::string passthrough_exe{ "findstr" };
static const std::vector<std::string> passthrough_args{ ".*" };
static const std::string stderr_exe{ "cmd" };
static const std::vector<std::string> stderr_args{ "/c", "echo error 1>&2" };
static const std::string stdout_stderr_exe{ "cmd" };
static const std::vector<std::string> stdout_stderr_args{ "/c", "echo out && echo err 1>&2" };
static const std::string invalid_exe{ "C:\\nonexistent\\binary\\that\\does\\not\\exist.exe" };
static const std::string temp_dir{ "C:\\Windows\\Temp" };
static const std::string invalid_dir{ "C:\\nonexistent\\path\\that\\does\\not\\exist" };
static const std::string false_exe{ "cmd" };
static const std::vector<std::string> false_args{ "/c", "exit 1" };
#else
static const std::string echo_exe{ "echo" };
static const std::vector<std::string> echo_hello{ "hello" };
static const std::string sleep_exe{ "sleep" };
static const std::vector<std::string> sleep_args{ "10" };
static const std::string passthrough_exe{ "cat" };
static const std::vector<std::string> passthrough_args{};
static const std::string stderr_exe{ "sh" };
static const std::vector<std::string> stderr_args{ "-c", "echo error >&2" };
static const std::string stdout_stderr_exe{ "sh" };
static const std::vector<std::string> stdout_stderr_args{ "-c", "echo out; echo err >&2" };
static const std::string invalid_exe{ "/nonexistent/binary/that/does/not/exist" };
static const std::string temp_dir{ "/tmp" };
static const std::string invalid_dir{ "/nonexistent/path/that/does/not/exist" };
static const std::string false_exe{ "false" };
static const std::vector<std::string> false_args{};
#endif

TEST(Process, StartSucceeds)
{
	process p{ echo_exe, echo_hello };
	ASSERT_TRUE(p.start());
	p.wait_for_exit();
}

TEST(Process, StartTwice)
{
	process p{ echo_exe, echo_hello };
	ASSERT_TRUE(p.start());
	ASSERT_FALSE(p.start());
	p.wait_for_exit();
}

TEST(Process, SetWorkingDirectoryAfterStart)
{
	process p{ echo_exe, echo_hello };
	p.start();
	p.wait_for_exit();
	ASSERT_FALSE(p.set_working_directory(temp_dir));
}

TEST(Process, SetWorkingDirectoryInvalidPath)
{
	process p{ echo_exe, echo_hello };
	p.set_working_directory(invalid_dir);
	ASSERT_FALSE(p.start());
}

TEST(Process, IsRunning)
{
	process p{ sleep_exe, sleep_args };
	ASSERT_TRUE(p.start());
	ASSERT_EQ(p.get_status(), process_status::running);
	p.kill();
	p.wait_for_exit();
}

TEST(Process, IsCompleted)
{
	process p{ echo_exe, echo_hello };
	p.start();
	p.wait_for_exit();
	ASSERT_EQ(p.get_status(), process_status::completed);
}

TEST(Process, ExitCodeZero)
{
	process p{ echo_exe, echo_hello };
	p.start();
	p.wait_for_exit();
	ASSERT_EQ(p.get_exit_code(), 0);
}

TEST(Process, ExitCodeNonZero)
{
	process p{ false_exe, false_args };
	p.start();
	p.wait_for_exit();
	ASSERT_NE(p.get_exit_code(), 0);
}

TEST(Process, WaitForExit)
{
	process p{ echo_exe, echo_hello };
	p.start();
	ASSERT_EQ(p.wait_for_exit(), 0);
}

TEST(Process, WaitForExitNoStart)
{
	process p{ echo_exe };
	ASSERT_EQ(p.wait_for_exit(), -1);
}

TEST(Process, Stdout)
{
	process p{ echo_exe, echo_hello };
	p.start();
	p.wait_for_exit();
	ASSERT_NE(p.get_standard_output().find("hello"), std::string::npos);
}

TEST(Process, Stderr)
{
	process p{ stderr_exe, stderr_args };
	p.start();
	p.wait_for_exit();
	ASSERT_NE(p.get_standard_error().find("error"), std::string::npos);
}

TEST(Process, StdoutAndStderr)
{
	process p{ stdout_stderr_exe, stdout_stderr_args };
	p.start();
	p.wait_for_exit();
	ASSERT_NE(p.get_standard_output().find("out"), std::string::npos);
	ASSERT_NE(p.get_standard_error().find("err"), std::string::npos);
	ASSERT_EQ(p.get_standard_output().find("err"), std::string::npos);
	ASSERT_EQ(p.get_standard_error().find("out"), std::string::npos);
}

TEST(Process, GetResult)
{
	process p{ echo_exe, echo_hello };
	p.start();
	p.wait_for_exit();
	process_result result{ p.get_result() };
	ASSERT_EQ(result.get_output(), p.get_standard_output());
	ASSERT_EQ(result.get_error(), p.get_standard_error());
	ASSERT_EQ(result.get_exit_code(), p.get_exit_code());
}

TEST(Process, Kill)
{
	process p{ sleep_exe, sleep_args };
	p.start();
	ASSERT_TRUE(p.kill());
	p.wait_for_exit();
	ASSERT_EQ(p.get_status(), process_status::killed);
}

TEST(Process, KillBeforeStart)
{
	process p{ sleep_exe, sleep_args };
	ASSERT_FALSE(p.kill());
}

TEST(Process, KillTwice)
{
	process p{ sleep_exe, sleep_args };
	p.start();
	ASSERT_TRUE(p.kill());
	p.wait_for_exit();
	ASSERT_FALSE(p.kill());
}

TEST(Process, Pause)
{
	process p{ sleep_exe, sleep_args };
	p.start();
	ASSERT_TRUE(p.pause());
	ASSERT_EQ(p.get_status(), process_status::paused);
	p.kill();
	p.wait_for_exit();
}

TEST(Process, PauseBeforeStart)
{
	process p{ sleep_exe, sleep_args };
	ASSERT_FALSE(p.pause());
}

TEST(Process, PauseTwice)
{
	process p{ sleep_exe, sleep_args };
	p.start();
	p.pause();
	ASSERT_FALSE(p.pause());
	p.kill();
	p.wait_for_exit();
}

TEST(Process, Resume)
{
	process p{ sleep_exe, sleep_args };
	p.start();
	p.pause();
	ASSERT_TRUE(p.resume());
	ASSERT_EQ(p.get_status(), process_status::running);
	p.kill();
	p.wait_for_exit();
}

TEST(Process, ResumeWithoutPause)
{
	process p{ sleep_exe, sleep_args };
	p.start();
	ASSERT_FALSE(p.resume());
	p.kill();
	p.wait_for_exit();
}

TEST(Process, WriteStdin)
{
	process p{ passthrough_exe, passthrough_args };
	p.start();
	ASSERT_TRUE(p.write("hello"));
	p.kill();
	p.wait_for_exit();
}

TEST(Process, WriteLineStdin)
{
	process p{ passthrough_exe, passthrough_args };
	p.start();
	ASSERT_TRUE(p.write_line("hello"));
	p.kill();
	p.wait_for_exit();
}

TEST(Process, WriteBeforeStart)
{
	process p{ passthrough_exe, passthrough_args };
	ASSERT_FALSE(p.write("hello"));
}

TEST(Process, WriteLineBeforeStart)
{
	process p{ passthrough_exe, passthrough_args };
	ASSERT_FALSE(p.write_line("hello"));
}

TEST(Process, StdinOutput)
{
	process p{ passthrough_exe, passthrough_args };
	p.start();
	ASSERT_TRUE(p.write_line("ping"));
#ifdef _WIN32
	p.write_line("");
#else
	p.kill();
#endif
	p.wait_for_exit();
	ASSERT_NE(p.get_standard_output().find("ping"), std::string::npos);
}

TEST(Process, Exited)
{
	process p{ echo_exe, echo_hello };
	bool fired{ false };
	p.get_exited_event() += [&fired](const process&, const param_event_args<int>&)
	{
		fired = true;
	};
	p.start();
	p.wait_for_exit();
	ASSERT_TRUE(fired);
}

TEST(Process, ExitedArgs)
{
	process p{ echo_exe, echo_hello };
	int received{ -99 };
	p.get_exited_event() += [&received](const process&, const param_event_args<int>& args)
	{
		received = *args;
	};
	p.start();
	p.wait_for_exit();
	ASSERT_EQ(received, 0);
}

TEST(Process, OutpuReceived)
{
	process p{ echo_exe, echo_hello };
	bool fired{ false };
	p.get_output_received_event() += [&fired](const process&, const param_event_args<std::string>& args)
	{
		if (!args->empty())
		{
			fired = true;
		}
	};
	p.start();
	p.wait_for_exit();
	ASSERT_TRUE(fired);
}

TEST(Process, ErrorReceived)
{
	process p{ stderr_exe, stderr_args };
	bool fired{ false };
	p.get_error_received_event() += [&fired](const process&, const param_event_args<std::string>& args)
	{
		if (!args->empty())
		{
			fired = true;
		}
	};
	p.start();
	p.wait_for_exit();
	ASSERT_TRUE(fired);
}

TEST(Process, StartInvalid)
{
	process p{ invalid_exe };
	p.start();
	int code{ p.wait_for_exit() };
	ASSERT_NE(code, 0);
}