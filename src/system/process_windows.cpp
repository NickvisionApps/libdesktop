#include "system/process.h"
#include <windows.h>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <tlhelp32.h>
#include <vector>

static constexpr std::chrono::milliseconds process_wait_timeout{ 50 };

static void close_handle(HANDLE& handle) noexcept
{
	if (handle)
	{
		CloseHandle(handle);
		handle = nullptr;
	}
}

static std::vector<DWORD> get_job_processes(HANDLE job)
{
	std::vector<DWORD> process_ids;
	std::size_t buffer_size{ sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + 255 * sizeof(ULONG_PTR) };
	JOBOBJECT_BASIC_PROCESS_ID_LIST* buffer{ reinterpret_cast<JOBOBJECT_BASIC_PROCESS_ID_LIST*>(std::malloc(buffer_size)) };
	if (!buffer)
	{
		return process_ids;
	}
	if (QueryInformationJobObject(job, JobObjectBasicProcessIdList, buffer, static_cast<DWORD>(buffer_size), nullptr))
	{
		process_ids.reserve(buffer->NumberOfProcessIdsInList);
		for (DWORD i = 0; i < buffer->NumberOfProcessIdsInList; ++i)
		{
			process_ids.push_back(static_cast<DWORD>(buffer->ProcessIdList[i]));
		}
	}
	std::free(buffer);
	return process_ids;
}

static std::wstring quote_argument(const std::wstring& value)
{
	if (value.empty())
	{
		return L"\"\"";
	}
	bool needs_quotes{ false };
	for (const wchar_t ch : value)
	{
		if (ch == L' ' || ch == L'\t' || ch == L'"')
		{
			needs_quotes = true;
			break;
		}
	}
	if (!needs_quotes)
	{
		return value;
	}
	std::wstring quoted;
	quoted.push_back(L'"');
	std::size_t backslash_count{ 0 };
	for (const wchar_t ch : value)
	{
		if (ch == L'\\')
		{
			++backslash_count;
			continue;
		}
		if (ch == L'"')
		{
			quoted.append(backslash_count * 2 + 1, L'\\');
			quoted.push_back(L'"');
			backslash_count = 0;
			continue;
		}
		if (backslash_count > 0)
		{
			quoted.append(backslash_count, L'\\');
			backslash_count = 0;
		}
		quoted.push_back(ch);
	}
	if (backslash_count > 0)
	{
		quoted.append(backslash_count * 2, L'\\');
	}
	quoted.push_back(L'"');
	return quoted;
}

static bool update_threads(HANDLE job, bool resume)
{
	std::vector<DWORD> process_ids{ get_job_processes(job) };
	HANDLE snapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0) };
	if (snapshot == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	THREADENTRY32 entry{};
	entry.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(snapshot, &entry))
	{
		do
		{
			for (DWORD process_id : process_ids)
			{
				if (entry.th32OwnerProcessID != process_id)
				{
					continue;
				}
				HANDLE thread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID) };
				if (!thread)
				{
					continue;
				}
				if (resume)
				{
					ResumeThread(thread);
				}
				else
				{
					SuspendThread(thread);
				}
				CloseHandle(thread);
				break;
			}
		} while (Thread32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return true;
}

static std::wstring utf8_to_wstring(const std::string& value)
{
	if (value.empty())
	{
		return {};
	}
	const int size{ MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0) };
	if (size <= 0)
	{
		return {};
	}
	std::wstring result(static_cast<std::size_t>(size), L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(), size) <= 0)
	{
		return {};
	}
	return result;
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
		HANDLE m_stdout_read;
		HANDLE m_stdout_write;
		HANDLE m_stderr_read;
		HANDLE m_stderr_write;
		HANDLE m_stdin_read;
		HANDLE m_stdin_write;
		HANDLE m_job;
		PROCESS_INFORMATION m_process_information;
		void append_pipe_output(std::string& buffer, HANDLE pipe);
		void cleanup() noexcept;
		void watch() noexcept;
	};

	process::impl::impl(process& process)
	    : m_process{ process },
	      m_status{ process_status::created },
	      m_exit_code{ -1 },
	      m_stdout_read{ nullptr },
	      m_stdout_write{ nullptr },
	      m_stderr_read{ nullptr },
	      m_stderr_write{ nullptr },
	      m_stdin_read{ nullptr },
	      m_stdin_write{ nullptr },
	      m_job{ nullptr },
	      m_process_information{}
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

	void process::impl::append_pipe_output(std::string& buffer, HANDLE pipe)
	{
		while (true)
		{
			DWORD available{ 0 };
			if (!PeekNamedPipe(pipe, nullptr, 0, nullptr, &available, nullptr) || available == 0)
			{
				return;
			}
			std::vector<char> chunk(available);
			DWORD read{ 0 };
			if (!ReadFile(pipe, chunk.data(), static_cast<DWORD>(chunk.size()), &read, nullptr) || read == 0)
			{
				return;
			}
			std::lock_guard<std::mutex> lock{ m_mutex };
			buffer.append(chunk.data(), read);
		}
	}

	void process::impl::cleanup() noexcept
	{
		close_handle(m_stdout_read);
		close_handle(m_stdout_write);
		close_handle(m_stderr_read);
		close_handle(m_stderr_write);
		close_handle(m_stdin_read);
		close_handle(m_stdin_write);
		close_handle(m_process_information.hProcess);
		close_handle(m_process_information.hThread);
		close_handle(m_job);
	}

	int process::impl::get_exit_code() const
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		return m_exit_code;
	}

	const std::string& process::impl::get_standard_error() const
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		return m_standard_error;
	}

	const std::string& process::impl::get_standard_output() const
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		return m_standard_output;
	}

	process_status process::impl::get_status() const
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		return m_status;
	}

	bool process::impl::input(std::string_view data) const
	{
		HANDLE stdin_write{ nullptr };
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			if (m_status != process_status::running)
			{
				return false;
			}
			stdin_write = m_stdin_write;
		}
		const char* current{ data.data() };
		std::size_t remaining{ data.size() };
		while (remaining > 0)
		{
			DWORD written{ 0 };
			const DWORD chunk_size{ remaining > static_cast<std::size_t>(MAXDWORD) ? MAXDWORD : static_cast<DWORD>(remaining) };
			if (!WriteFile(stdin_write, current, chunk_size, &written, nullptr))
			{
				return false;
			}
			current += written;
			remaining -= written;
		}
		return true;
	}

	bool process::impl::kill()
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		if (m_status != process_status::running && m_status != process_status::paused)
		{
			return false;
		}
		if (!TerminateJobObject(m_job, 1))
		{
			return false;
		}
		m_status = process_status::killed;
		return true;
	}

	bool process::impl::pause()
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		if (m_status != process_status::running)
		{
			return false;
		}
		bool res{ update_threads(m_job, false) };
		m_status = process_status::paused;
		return res;
	}

	bool process::impl::resume()
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		if (m_status != process_status::paused)
		{
			return false;
		}
		bool res{ update_threads(m_job, true) };
		m_status = process_status::running;
		return res;
	}

	bool process::impl::start()
	{
		std::lock_guard<std::mutex> lock{ m_mutex };
		if (m_status != process_status::created)
		{
			return false;
		}
		if (!m_process.m_working_directory.empty() &&
		    (!std::filesystem::exists(m_process.m_working_directory) || !std::filesystem::is_directory(m_process.m_working_directory)))
		{
			return false;
		}
		try
		{
			SECURITY_ATTRIBUTES security_attributes{};
			security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
			security_attributes.bInheritHandle = TRUE;
			security_attributes.lpSecurityDescriptor = nullptr;
			if (!CreatePipe(&m_stdout_read, &m_stdout_write, &security_attributes, 0) || !SetHandleInformation(m_stdout_read, HANDLE_FLAG_INHERIT, 0) ||
			    !CreatePipe(&m_stderr_read, &m_stderr_write, &security_attributes, 0) || !SetHandleInformation(m_stderr_read, HANDLE_FLAG_INHERIT, 0) ||
			    !CreatePipe(&m_stdin_read, &m_stdin_write, &security_attributes, 0) || !SetHandleInformation(m_stdin_write, HANDLE_FLAG_INHERIT, 0))
			{
				cleanup();
				return false;
			}
			m_job = CreateJobObjectW(nullptr, nullptr);
			if (!m_job)
			{
				cleanup();
				return false;
			}
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
			limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
			if (!SetInformationJobObject(m_job, JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
			{
				cleanup();
				return false;
			}
			STARTUPINFOW startup_info{};
			startup_info.cb = sizeof(STARTUPINFOW);
			startup_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			startup_info.wShowWindow = SW_HIDE;
			startup_info.hStdOutput = m_stdout_write;
			startup_info.hStdError = m_stderr_write;
			startup_info.hStdInput = m_stdin_read;
			std::wstring command_line{ quote_argument(m_process.m_path.wstring()) };
			for (const std::string& argument : m_process.m_arguments)
			{
				command_line += L" ";
				command_line += quote_argument(utf8_to_wstring(argument));
			}
			std::vector<wchar_t> command_line_buffer(command_line.begin(), command_line.end());
			command_line_buffer.push_back(L'\0');
			const std::wstring working_directory_str{ m_process.m_working_directory.empty() ? std::wstring{} : m_process.m_working_directory.wstring() };
			if (!CreateProcessW(nullptr, command_line_buffer.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, nullptr,
			                    working_directory_str.empty() ? nullptr : working_directory_str.c_str(), &startup_info, &m_process_information))
			{
				cleanup();
				return false;
			}
			close_handle(m_stdout_write);
			close_handle(m_stderr_write);
			close_handle(m_stdin_read);
			if (!AssignProcessToJobObject(m_job, m_process_information.hProcess))
			{
				TerminateProcess(m_process_information.hProcess, 1);
				return false;
			}
			m_status = process_status::running;
			m_exit_code = -1;
			m_standard_output.clear();
			m_standard_error.clear();
			m_watch_thread = std::thread(&impl::watch, this);
		}
		catch (...)
		{
			if (m_process_information.hProcess)
			{
				TerminateProcess(m_process_information.hProcess, 1);
			}
			cleanup();
			m_status = process_status::created;
			m_exit_code = -1;
			return false;
		}
		return true;
	}

	int process::impl::wait_for_exit()
	{
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			if (m_status == process_status::created)
			{
				return -1;
			}
		}
		if (m_watch_thread.joinable())
		{
			m_watch_thread.join();
		}
		std::lock_guard<std::mutex> lock{ m_mutex };
		return m_exit_code;
	}

	void process::impl::watch() noexcept
	{
		DWORD wait_result{ WAIT_TIMEOUT };
		DWORD process_exit_code{ STILL_ACTIVE };
		while (true)
		{
			wait_result = WaitForSingleObject(m_process_information.hProcess, static_cast<DWORD>(process_wait_timeout.count()));
			append_pipe_output(m_standard_output, m_stdout_read);
			append_pipe_output(m_standard_error, m_stderr_read);
			if (wait_result == WAIT_OBJECT_0)
			{
				break;
			}
			if (wait_result == WAIT_FAILED)
			{
				process_exit_code = static_cast<DWORD>(-1);
				break;
			}
		}
		append_pipe_output(m_standard_output, m_stdout_read);
		append_pipe_output(m_standard_error, m_stderr_read);
		int exit_code;
		{
			std::lock_guard<std::mutex> lock{ m_mutex };
			if (process_exit_code == STILL_ACTIVE && !GetExitCodeProcess(m_process_information.hProcess, &process_exit_code))
			{
				process_exit_code = static_cast<DWORD>(-1);
			}
			m_exit_code = static_cast<int>(process_exit_code);
			if (m_status != process_status::killed)
			{
				m_status = process_status::completed;
			}
			exit_code = m_exit_code;
		}
		m_process.m_exited_event.invoke(m_process, { exit_code });
	}

	process::process(const std::filesystem::path& path, const std::vector<std::string>& arguments)
	    : m_path{ path },
	      m_arguments{ arguments },
	      m_impl{ std::make_unique<impl>(*this) }
	{
	}

	process::~process() = default;

	const events::event<process, events::param_event_args<int>>& process::get_exited_event() const
	{
		return m_exited_event;
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
		return m_impl->input(data + "\r\n");
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
