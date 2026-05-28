#include "system/process.h"
#include <array>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

static constexpr std::chrono::milliseconds process_wait_timeout{ 50 };

static void close_fd(int& fd) noexcept
{
	if (fd >= 0)
	{
		close(fd);
		fd = -1;
	}
}

static std::string read_available(int fd, std::string& buffer)
{
	std::size_t old_size{ buffer.size() };
	std::array<char, 4096> chunk{};
	while (true)
	{
		ssize_t bytes{ ::read(fd, chunk.data(), chunk.size()) };
		if (bytes > 0)
		{
			buffer.append(chunk.data(), static_cast<std::size_t>(bytes));
			continue;
		}
		if (bytes == 0 || errno == EAGAIN || errno == EWOULDBLOCK)
		{
			break;
		}
		if (errno != EINTR)
		{
			break;
		}
	}
	return buffer.substr(old_size);
}

static bool send_signal(pid_t pid, int signal)
{
	bool delivered{ false };
	if (::kill(-pid, signal) == 0)
	{
		delivered = true;
	}
	else if (errno != ESRCH)
	{
		return false;
	}
	if (::kill(pid, signal) == 0)
	{
		delivered = true;
	}
	else if (errno != ESRCH)
	{
		return false;
	}
	return delivered;
}

namespace desktop::system
{
	class process::state
	{
	public:
		pid_t pid{ -1 };
		std::array<int, 2> stdout_pipe{ -1, -1 };
		std::array<int, 2> stderr_pipe{ -1, -1 };
		std::array<int, 2> stdin_pipe{ -1, -1 };
	};

	process::process(std::filesystem::path path, std::vector<std::string> arguments)
	    : m_state{ std::make_unique<state>() },
	      m_path{ std::move(path) },
	      m_arguments{ std::move(arguments) }
	{
	}

	process::~process()
	{
		if (m_watcher.joinable())
		{
			m_watcher.join();
		}
		close_fd(m_state->stdout_pipe[0]);
		close_fd(m_state->stdout_pipe[1]);
		close_fd(m_state->stderr_pipe[0]);
		close_fd(m_state->stderr_pipe[1]);
		close_fd(m_state->stdin_pipe[0]);
		close_fd(m_state->stdin_pipe[1]);
	}

	const events::event<process, events::param_event_args<int>>& process::get_exited_event() const
	{
		return m_exited_event;
	}

	const events::event<process, events::param_event_args<std::string>>& process::get_output_received_event() const
	{
		return m_output_received_event;
	}

	const events::event<process, events::param_event_args<std::string>>& process::get_error_received_event() const
	{
		return m_error_received_event;
	}

	const std::vector<std::string>& process::get_arguments() const
	{
		return m_arguments;
	}

	int process::get_exit_code() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_exit_code;
	}

	const std::filesystem::path& process::get_path() const
	{
		return m_path;
	}

	std::string process::get_standard_error() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_standard_error;
	}

	std::string process::get_standard_output() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_standard_output;
	}

	process_result process::get_result() const
	{
		std::scoped_lock lock{ m_mutex };
		return { m_standard_output, m_standard_error, m_exit_code };
	}

	process_status process::get_status() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_status;
	}

	const std::filesystem::path& process::get_working_directory() const
	{
		return m_working_directory;
	}

	bool process::write(std::string_view data) const
	{
		std::unique_lock lock{ m_mutex };
		int fd{ -1 };
		if (m_status != process_status::running)
		{
			return false;
		}
		fd = m_state->stdin_pipe[1];
		lock.unlock();
		const char* p{ data.data() };
		std::size_t remaining{ data.size() };
		while (remaining > 0)
		{
			ssize_t written{ ::write(fd, p, remaining) };
			if (written < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				return false;
			}
			p += written;
			remaining -= static_cast<std::size_t>(written);
		}
		return true;
	}

	bool process::write_line(const std::string& data) const
	{
		return write(data + "\n");
	}

	bool process::kill()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status == process_status::created || m_status == process_status::completed || m_status == process_status::killed)
		{
			return false;
		}
		if (m_state->pid <= 0)
		{
			return false;
		}
		if (!send_signal(m_state->pid, SIGTERM))
		{
			return false;
		}
		m_status = process_status::killed;
		return true;
	}

	bool process::pause()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::running)
		{
			return false;
		}
		if (m_state->pid <= 0)
		{
			return false;
		}
		if (!send_signal(m_state->pid, SIGSTOP))
		{
			return false;
		}
		m_status = process_status::paused;
		return true;
	}

	bool process::resume()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::paused)
		{
			return false;
		}
		if (m_state->pid <= 0)
		{
			return false;
		}
		if (!send_signal(m_state->pid, SIGCONT))
		{
			return false;
		}
		m_status = process_status::running;
		return true;
	}

	bool process::set_working_directory(const std::filesystem::path& path)
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::created)
		{
			return false;
		}
		m_working_directory = path;
		return true;
	}

	bool process::start()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::created)
		{
			return false;
		}
		if (!m_working_directory.empty() && (!std::filesystem::exists(m_working_directory) || !std::filesystem::is_directory(m_working_directory)))
		{
			return false;
		}
		if (pipe(m_state->stdout_pipe.data()) < 0 || pipe(m_state->stderr_pipe.data()) < 0 || pipe(m_state->stdin_pipe.data()) < 0)
		{
			close_fd(m_state->stdout_pipe[0]);
			close_fd(m_state->stdout_pipe[1]);
			close_fd(m_state->stderr_pipe[0]);
			close_fd(m_state->stderr_pipe[1]);
			close_fd(m_state->stdin_pipe[0]);
			close_fd(m_state->stdin_pipe[1]);
			return false;
		}
		m_state->pid = fork();
		if (m_state->pid < 0)
		{
			close_fd(m_state->stdout_pipe[0]);
			close_fd(m_state->stdout_pipe[1]);
			close_fd(m_state->stderr_pipe[0]);
			close_fd(m_state->stderr_pipe[1]);
			close_fd(m_state->stdin_pipe[0]);
			close_fd(m_state->stdin_pipe[1]);
			return false;
		}
		if (m_state->pid == 0)
		{
			setpgid(0, 0);
			dup2(m_state->stdout_pipe[1], STDOUT_FILENO);
			dup2(m_state->stderr_pipe[1], STDERR_FILENO);
			dup2(m_state->stdin_pipe[0], STDIN_FILENO);
			close_fd(m_state->stdout_pipe[0]);
			close_fd(m_state->stdout_pipe[1]);
			close_fd(m_state->stderr_pipe[0]);
			close_fd(m_state->stderr_pipe[1]);
			close_fd(m_state->stdin_pipe[0]);
			close_fd(m_state->stdin_pipe[1]);
			if (!m_working_directory.empty())
			{
				chdir(m_working_directory.string().c_str());
			}
			std::vector<std::string> args{ m_arguments };
			std::vector<char*> argv;
			argv.reserve(args.size() + 2);
			std::string exe{ m_path.string() };
			argv.push_back(exe.data());
			for (auto& a : args)
			{
				argv.push_back(a.data());
			}
			argv.push_back(nullptr);
			execvp(m_path.string().c_str(), argv.data());
			_exit(127);
		}
		setpgid(m_state->pid, m_state->pid);
		close_fd(m_state->stdout_pipe[1]);
		close_fd(m_state->stderr_pipe[1]);
		close_fd(m_state->stdin_pipe[0]);
		int flags{ fcntl(m_state->stdout_pipe[0], F_GETFL, 0) };
		if (flags >= 0)
		{
			fcntl(m_state->stdout_pipe[0], F_SETFL, flags | O_NONBLOCK);
		}
		flags = fcntl(m_state->stderr_pipe[0], F_GETFL, 0);
		if (flags >= 0)
		{
			fcntl(m_state->stderr_pipe[0], F_SETFL, flags | O_NONBLOCK);
		}
		m_status = process_status::running;
		m_exit_code = -1;
		m_standard_output.clear();
		m_standard_error.clear();
		m_watcher = std::thread(&process::watch, this);
		return true;
	}

	int process::wait_for_exit()
	{
		std::unique_lock lock{ m_mutex };
		if (m_status == process_status::created)
		{
			return -1;
		}
		lock.unlock();
		if (m_watcher.joinable())
		{
			m_watcher.join();
		}
		lock.lock();
		return m_exit_code;
	}

	void process::watch()
	{
		int status_value{ 0 };
		bool exited{ false };
		while (!exited)
		{
			pid_t r{ waitpid(m_state->pid, &status_value, WNOHANG | WUNTRACED | WCONTINUED) };
			if (r == m_state->pid)
			{
				if (WIFEXITED(status_value) || WIFSIGNALED(status_value))
				{
					exited = true;
				}
			}
			std::unique_lock lock{ m_mutex };
			std::string new_output{ read_available(m_state->stdout_pipe[0], m_standard_output) };
			std::string new_error{ read_available(m_state->stderr_pipe[0], m_standard_error) };
			lock.unlock();
			if (!new_output.empty())
			{
				m_output_received_event.invoke(*this, { new_output });
			}
			if (!new_error.empty())
			{
				m_error_received_event.invoke(*this, { new_error });
			}
			if (!exited)
			{
				std::this_thread::sleep_for(process_wait_timeout);
			}
		}
		std::unique_lock lock{ m_mutex };
		std::string new_output{ read_available(m_state->stdout_pipe[0], m_standard_output) };
		std::string new_error{ read_available(m_state->stderr_pipe[0], m_standard_error) };
		lock.unlock();
		if (!new_output.empty())
		{
			m_output_received_event.invoke(*this, { new_output });
		}
		if (!new_error.empty())
		{
			m_error_received_event.invoke(*this, { new_error });
		}
		lock.lock();
		m_exit_code = WIFEXITED(status_value) ? WEXITSTATUS(status_value) : -1;
		if (m_status != process_status::killed)
		{
			m_status = process_status::completed;
		}
		lock.unlock();
		m_exited_event.invoke(*this, { m_exit_code });
	}
}