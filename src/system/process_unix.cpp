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
	int flags{ fcntl(fd, F_GETFL, 0) };
	if (flags < 0)
	{
		return;
	}
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
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
		process_status m_status{ process_status::created };
		int m_exit_code{ -1 };
		std::string m_standard_output;
		std::string m_standard_error;
		std::thread m_watch_thread;
		pid_t m_pid{ -1 };
		std::array<int, 2> m_stdout_pipe{ -1, -1 };
		std::array<int, 2> m_stderr_pipe{ -1, -1 };
		std::array<int, 2> m_stdin_pipe{ -1, -1 };
		void cleanup() noexcept;
		std::string read_available(int fd, std::string& buffer);
		void watch() noexcept;
	};

	process::impl::impl(process& process)
	    : m_process{ process }
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
		int fd{ -1 };
		{
			std::scoped_lock lock{ m_mutex };
			if (m_status != process_status::running)
			{
				return false;
			}
			fd = m_stdin_pipe[1];
		}
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

	bool process::impl::kill()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status == process_status::created || m_status == process_status::completed || m_status == process_status::killed)
		{
			return false;
		}
		if (m_pid <= 0)
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
		if (m_pid <= 0)
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

	bool process::impl::resume()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::paused)
		{
			return false;
		}
		if (m_pid <= 0)
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

	std::string process::impl::read_available(int fd, std::string& buffer)
	{
		std::size_t old_size{ buffer.size() };
		std::array<char, 4096> chunk{};
		while (true)
		{
			ssize_t bytes{ ::read(fd, chunk.data(), chunk.size()) };
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
		if (pipe(m_stdout_pipe.data()) < 0 || pipe(m_stderr_pipe.data()) < 0 || pipe(m_stdin_pipe.data()) < 0)
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
			dup2(m_stdout_pipe[1], STDOUT_FILENO);
			dup2(m_stderr_pipe[1], STDERR_FILENO);
			dup2(m_stdin_pipe[0], STDIN_FILENO);
			cleanup();
			if (!m_process.m_working_directory.empty())
			{
				chdir(m_process.m_working_directory.string().c_str());
			}
			std::vector<std::string> args{ m_process.m_arguments };
			std::vector<char*> argv;
			argv.reserve(args.size() + 2);
			std::string exe{ m_process.m_path.string() };
			argv.push_back(exe.data());
			for (auto& a : args)
			{
				argv.push_back(a.data());
			}
			argv.push_back(nullptr);
			execvp(m_process.m_path.string().c_str(), argv.data());
			_exit(127);
		}
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
		int status_value{ 0 };
		bool exited{ false };
		while (!exited)
		{
			pid_t r{ waitpid(m_pid, &status_value, WNOHANG | WUNTRACED | WCONTINUED) };
			if (r == m_pid)
			{
				std::scoped_lock lock{ m_mutex };
				if (WIFEXITED(status_value) || WIFSIGNALED(status_value))
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
		waitpid(m_pid, &status_value, 0);
		{
			std::scoped_lock lock{ m_mutex };
			m_exit_code = WIFEXITED(status_value) ? WEXITSTATUS(status_value) : -1;
			if (m_status != process_status::killed)
			{
				m_status = process_status::completed;
			}
		}
		m_process.m_exited_event.invoke(m_process, { m_exit_code });
	}

	process::process(std::filesystem::path path, std::vector<std::string> arguments)
	    : m_impl{ std::make_unique<impl>(*this) },
	      m_path{ std::move(path) },
	      m_arguments{ std::move(arguments) }
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

	process_result process::get_result() const
	{
		return { m_impl->get_standard_output(), m_impl->get_standard_error(), m_impl->get_exit_code() };
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