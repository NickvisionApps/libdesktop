#include "system/process.h"
#include <array>
#include <chrono>
#include <csignal>
#include <cstring>
#include <errno.h>
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

static void set_nonblocking(int fd)
{
	const int flags{ fcntl(fd, F_GETFL, 0) };
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		return;
	}
}

namespace desktop::system
{
	class process::impl
	{
	public:
		impl(process& process);
		~impl();
		impl(const impl&) = delete;
		impl(impl&&) = delete;
		impl& operator=(const impl&) = delete;
		impl& operator=(impl&&) = delete;
		int get_exit_code() const;
		const std::string& get_standard_error() const;
		const std::string& get_standard_output() const;
		process_status get_status() const;
		bool input(std::string_view data) const;
		bool kill();
		bool pause();
		bool resume();
		bool start();
		int wait_for_exit();

	private:
		mutable std::mutex m_mutex;
		process& m_process;
		process_status m_status;
		int m_exit_code;
		std::string m_standard_output;
		std::string m_standard_error;
		std::thread m_watch_thread;
		pid_t m_pid;
		int m_stdout_pipe[2];
		int m_stderr_pipe[2];
		int m_stdin_pipe[2];
		void cleanup() noexcept;
		std::string read_available(int fd, std::string& buffer);
		void watch() noexcept;
	};

	process::impl::impl(process& process)
	    : m_process{ process },
	      m_status{ process_status::created },
	      m_exit_code{ -1 },
	      m_pid{ -1 },
	      m_stdout_pipe{ -1, -1 },
	      m_stderr_pipe{ -1, -1 },
	      m_stdin_pipe{ -1, -1 }
	{
	}

	process::impl::~impl()
	{
		if (m_watch_thread.joinable())
		{
			m_watch_thread.join();
		}
		cleanup();
	}

	void process::impl::cleanup() noexcept
	{
		close_fd(m_stdout_pipe[0]);
		close_fd(m_stdout_pipe[1]);
		close_fd(m_stderr_pipe[0]);
		close_fd(m_stderr_pipe[1]);
		close_fd(m_stdin_pipe[0]);
		close_fd(m_stdin_pipe[1]);
	}

	int process::impl::get_exit_code() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_exit_code;
	}

	const std::string& process::impl::get_standard_error() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_standard_error;
	}

	const std::string& process::impl::get_standard_output() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_standard_output;
	}

	process_status process::impl::get_status() const
	{
		std::scoped_lock lock{ m_mutex };
		return m_status;
	}

	bool process::impl::input(std::string_view data) const
	{
		int stdin_fd{ -1 };
		{
			std::scoped_lock lock{ m_mutex };
			if (m_status != process_status::running)
			{
				return false;
			}
			stdin_fd = m_stdin_pipe[1];
		}
		const char* current{ data.data() };
		std::size_t remaining{ data.size() };
		while (remaining > 0)
		{
			const ssize_t written{ write(stdin_fd, current, remaining) };
			if (written < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				return false;
			}
			current += written;
			remaining -= static_cast<std::size_t>(written);
		}
		return true;
	}

	bool process::impl::kill()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::running && m_status != process_status::paused)
		{
			return false;
		}
		if (!send_signal(m_pid, SIGTERM))
		{
			return false;
		}
		m_status = process_status::killed;
		return true;
	}

	bool process::impl::pause()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::running)
		{
			return false;
		}
		if (!send_signal(m_pid, SIGSTOP))
		{
			return false;
		}
		m_status = process_status::paused;
		return true;
	}

	std::string process::impl::read_available(int fd, std::string& buffer)
	{
		size_t old_size{ buffer.size() };
		std::array<char, 4096> chunk{};
		while (true)
		{
			const ssize_t bytes{ read(fd, chunk.data(), chunk.size()) };
			if (bytes > 0)
			{
				std::scoped_lock lock{ m_mutex };
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

	bool process::impl::resume()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::paused)
		{
			return false;
		}
		if (!send_signal(m_pid, SIGCONT))
		{
			return false;
		}
		m_status = process_status::running;
		return true;
	}

	bool process::impl::start()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::created)
		{
			return false;
		}
		if (!m_process.m_working_directory.empty() &&
		    (!std::filesystem::exists(m_process.m_working_directory) || !std::filesystem::is_directory(m_process.m_working_directory)))
		{
			return false;
		}
		if (pipe(m_stdout_pipe) < 0 || pipe(m_stderr_pipe) < 0 || pipe(m_stdin_pipe) < 0)
		{
			cleanup();
			return false;
		}
		m_pid = fork();
		if (m_pid < 0)
		{
			cleanup();
			return false;
		}
		if (m_pid == 0)
		{
			setpgid(0, 0);
			if (dup2(m_stdout_pipe[1], STDOUT_FILENO) < 0 || dup2(m_stderr_pipe[1], STDERR_FILENO) < 0 || dup2(m_stdin_pipe[0], STDIN_FILENO) < 0)
			{
				const std::string message{ std::string("Failed to configure process pipes: ") + std::strerror(errno) + "\n" };
				write(STDERR_FILENO, message.data(), message.size());
				_exit(127);
			}
			cleanup();
			if (!m_process.m_working_directory.empty() && chdir(m_process.m_working_directory.string().c_str()) != 0)
			{
				const std::string message{ std::string("Failed to change the working directory: ") + std::strerror(errno) + "\n" };
				write(STDERR_FILENO, message.data(), message.size());
				_exit(127);
			}
			std::vector<std::string> args{ m_process.m_arguments };
			std::vector<char*> argv;
			argv.reserve(args.size() + 2);
			std::string executable{ m_process.m_path.string() };
			argv.push_back(executable.data());
			for (std::string& arg : args)
			{
				argv.push_back(arg.data());
			}
			argv.push_back(nullptr);
			execvp(m_process.m_path.string().c_str(), argv.data());
			const std::string message{ std::string("Failed to execute process: ") + std::strerror(errno) + "\n" };
			write(STDERR_FILENO, message.data(), message.size());
			_exit(127);
		}
		try
		{
			setpgid(m_pid, m_pid);
			close_fd(m_stdout_pipe[1]);
			close_fd(m_stderr_pipe[1]);
			close_fd(m_stdin_pipe[0]);
			set_nonblocking(m_stdout_pipe[0]);
			set_nonblocking(m_stderr_pipe[0]);
			m_status = process_status::running;
			m_exit_code = -1;
			m_standard_output.clear();
			m_standard_error.clear();
			m_watch_thread = std::thread(&impl::watch, this);
		}
		catch (...)
		{
			send_signal(m_pid, SIGTERM);
			cleanup();
			m_pid = -1;
			m_status = process_status::created;
			m_exit_code = -1;
			return false;
		}
		return true;
	}

	int process::impl::wait_for_exit()
	{
		{
			std::scoped_lock lock{ m_mutex };
			if (m_status == process_status::created)
			{
				return -1;
			}
		}
		if (m_watch_thread.joinable())
		{
			m_watch_thread.join();
		}
		std::scoped_lock lock{ m_mutex };
		return m_exit_code;
	}

	void process::impl::watch() noexcept
	{
		try
		{
			int status_value{ 0 };
			bool exited{ false };
			while (!exited)
			{
				const pid_t result{ waitpid(m_pid, &status_value, WNOHANG | WUNTRACED | WCONTINUED) };
				if (result == m_pid)
				{
					std::scoped_lock lock{ m_mutex };
					if (WIFSTOPPED(status_value))
					{
						m_status = process_status::paused;
					}
					else if (WIFCONTINUED(status_value))
					{
						if (m_status != process_status::killed)
						{
							m_status = process_status::running;
						}
					}
					else if (WIFEXITED(status_value) || WIFSIGNALED(status_value))
					{
						exited = true;
					}
				}
				m_process.m_output_received_event.invoke(m_process, { read_available(m_stdout_pipe[0], m_standard_output) });
				m_process.m_error_received_event.invoke(m_process, { read_available(m_stderr_pipe[0], m_standard_error) });
				if (!exited)
				{
					std::this_thread::sleep_for(process_wait_timeout);
				}
			}
			m_process.m_output_received_event.invoke(m_process, { read_available(m_stdout_pipe[0], m_standard_output) });
			m_process.m_error_received_event.invoke(m_process, { read_available(m_stderr_pipe[0], m_standard_error) });
			int exit_code;
			{
				std::scoped_lock lock{ m_mutex };
				m_exit_code = WIFEXITED(status_value) ? WEXITSTATUS(status_value) : -1;
				if (m_status != process_status::killed)
				{
					m_status = process_status::completed;
				}
				exit_code = m_exit_code;
			}
			m_process.m_exited_event.invoke(m_process, { exit_code });
		}
		catch (...)
		{
		}
	}

	process::process(std::filesystem::path path, std::vector<std::string> arguments)
	    : m_path{ std::move(path) },
	      m_arguments{ std::move(arguments) },
	      m_impl{ std::make_unique<impl>(*this) }
	{
	}

	process::~process() = default;

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
		return m_impl->get_exit_code();
	}

	const std::filesystem::path& process::get_path() const
	{
		return m_path;
	}

	const std::string& process::get_standard_error() const
	{
		return m_impl->get_standard_error();
	}

	const std::string& process::get_standard_output() const
	{
		return m_impl->get_standard_output();
	}

	process_status process::get_status() const
	{
		return m_impl->get_status();
	}

	const std::filesystem::path& process::get_working_directory() const
	{
		return m_working_directory;
	}

	bool process::write(std::string_view data) const
	{
		return m_impl->input(data);
	}

	bool process::write_line(const std::string& data) const
	{
		return m_impl->input(data + "\n");
	}

	bool process::kill()
	{
		return m_impl->kill();
	}

	bool process::pause()
	{
		return m_impl->pause();
	}

	bool process::resume()
	{
		return m_impl->resume();
	}

	bool process::set_working_directory(const std::filesystem::path& path)
	{
		if (m_impl->get_status() != process_status::created)
		{
			return false;
		}
		m_working_directory = path;
		return true;
	}

	bool process::start()
	{
		return m_impl->start();
	}

	int process::wait_for_exit() const
	{
		return m_impl->wait_for_exit();
	}
}
