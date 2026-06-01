#include "system/process.h"
#include <windows.h>
#include <array>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <span>
#include <thread>
#include <tlhelp32.h>
#include <vector>
#include "helpers/string_manip.h"

using namespace desktop::helpers;

static void close_handle(HANDLE& handle) noexcept
{
	if (handle != nullptr)
	{
		CloseHandle(handle);
		handle = nullptr;
	}
}

static std::vector<DWORD> get_job_processes(HANDLE job)
{
	std::vector<DWORD> process_ids;
	std::size_t buffer_size{ sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + (255 * sizeof(ULONG_PTR)) };
	std::vector<std::byte> buffer_storage(buffer_size);
	JOBOBJECT_BASIC_PROCESS_ID_LIST* buffer{ reinterpret_cast<JOBOBJECT_BASIC_PROCESS_ID_LIST*>(buffer_storage.data()) };
	if (QueryInformationJobObject(job, JobObjectBasicProcessIdList, buffer, static_cast<DWORD>(buffer_size), nullptr) != FALSE)
	{
		std::span<const ULONG_PTR> process_list{ buffer->ProcessIdList, buffer->NumberOfProcessIdsInList };
		process_ids.reserve(process_list.size());
		for (ULONG_PTR pid : process_list)
		{
			process_ids.push_back(static_cast<DWORD>(pid));
		}
	}
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
			quoted.append((backslash_count * 2) + 1, L'\\');
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
	for (bool has_entry{ Thread32First(snapshot, &entry) != FALSE }; has_entry; has_entry = (Thread32Next(snapshot, &entry) != FALSE))
	{
		for (DWORD process_id : process_ids)
		{
			if (entry.th32OwnerProcessID != process_id)
			{
				continue;
			}
			HANDLE thread{ OpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID) };
			if (thread == nullptr)
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
	}
	CloseHandle(snapshot);
	return true;
}

static std::string append_pipe_output(std::string& buffer, HANDLE pipe)
{
	std::size_t old_size{ buffer.size() };
	while (true)
	{
		DWORD available{ 0 };
		if (PeekNamedPipe(pipe, nullptr, 0, nullptr, &available, nullptr) == FALSE || available == 0)
		{
			break;
		}
		std::vector<char> chunk(available);
		DWORD read{ 0 };
		if (ReadFile(pipe, chunk.data(), static_cast<DWORD>(chunk.size()), &read, nullptr) == FALSE || read == 0)
		{
			break;
		}
		buffer.append(chunk.data(), read);
	}
	return buffer.substr(old_size);
}

namespace desktop::system
{
	class process::state
	{
	public:
		HANDLE stdout_read{ nullptr };
		HANDLE stdout_write{ nullptr };
		HANDLE stderr_read{ nullptr };
		HANDLE stderr_write{ nullptr };
		HANDLE stdin_read{ nullptr };
		HANDLE stdin_write{ nullptr };
		HANDLE job{ nullptr };
		PROCESS_INFORMATION process_information{};
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
		close_handle(m_state->stdout_read);
		close_handle(m_state->stdout_write);
		close_handle(m_state->stderr_read);
		close_handle(m_state->stderr_write);
		close_handle(m_state->stdin_read);
		close_handle(m_state->stdin_write);
		close_handle(m_state->process_information.hProcess);
		close_handle(m_state->process_information.hThread);
		close_handle(m_state->job);
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
		HANDLE stdin_write{ nullptr };
		if (m_status != process_status::running)
		{
			return false;
		}
		stdin_write = m_state->stdin_write;
		lock.unlock();
		const char* current{ data.data() };
		std::size_t remaining{ data.size() };
		while (remaining > 0)
		{
			DWORD written{ 0 };
			DWORD chunk_size{ remaining > static_cast<std::size_t>(MAXDWORD) ? MAXDWORD : static_cast<DWORD>(remaining) };
			if (WriteFile(stdin_write, current, chunk_size, &written, nullptr) == FALSE)
			{
				return false;
			}
			current += written;
			remaining -= written;
		}
		return true;
	}

	bool process::write_line(const std::string& data) const
	{
		return write(data + "\r\n");
	}

	bool process::kill()
	{
		std::unique_lock lock{ m_mutex };
		if (m_status != process_status::running && m_status != process_status::paused)
		{
			return false;
		}
		lock.unlock();
		close_handle(m_state->stdin_write);
		if (TerminateJobObject(m_state->job, 1) == FALSE)
		{
			return false;
		}
		lock.lock();
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
		bool res{ update_threads(m_state->job, false) };
		m_status = process_status::paused;
		return res;
	}

	bool process::resume()
	{
		std::scoped_lock lock{ m_mutex };
		if (m_status != process_status::paused)
		{
			return false;
		}
		bool res{ update_threads(m_state->job, true) };
		m_status = process_status::running;
		return res;
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
		try
		{
			SECURITY_ATTRIBUTES security_attributes{ .nLength = sizeof(SECURITY_ATTRIBUTES), .lpSecurityDescriptor = nullptr, .bInheritHandle = TRUE };
			if (CreatePipe(&m_state->stdout_read, &m_state->stdout_write, &security_attributes, 0) == FALSE ||
			    SetHandleInformation(m_state->stdout_read, HANDLE_FLAG_INHERIT, 0) == FALSE ||
			    CreatePipe(&m_state->stderr_read, &m_state->stderr_write, &security_attributes, 0) == FALSE ||
			    SetHandleInformation(m_state->stderr_read, HANDLE_FLAG_INHERIT, 0) == FALSE ||
			    CreatePipe(&m_state->stdin_read, &m_state->stdin_write, &security_attributes, 0) == FALSE ||
			    SetHandleInformation(m_state->stdin_write, HANDLE_FLAG_INHERIT, 0) == FALSE)
			{
				close_handle(m_state->stdout_read);
				close_handle(m_state->stdout_write);
				close_handle(m_state->stderr_read);
				close_handle(m_state->stderr_write);
				close_handle(m_state->stdin_read);
				close_handle(m_state->stdin_write);
				return false;
			}
			m_state->job = CreateJobObjectW(nullptr, nullptr);
			if (m_state->job == nullptr)
			{
				close_handle(m_state->stdout_read);
				close_handle(m_state->stdout_write);
				close_handle(m_state->stderr_read);
				close_handle(m_state->stderr_write);
				close_handle(m_state->stdin_read);
				close_handle(m_state->stdin_write);
				return false;
			}
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{ .BasicLimitInformation = { .LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
				                                                                                  JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION } };
			if (SetInformationJobObject(m_state->job, JobObjectExtendedLimitInformation, &limits, sizeof(limits)) == FALSE)
			{
				close_handle(m_state->stdout_read);
				close_handle(m_state->stdout_write);
				close_handle(m_state->stderr_read);
				close_handle(m_state->stderr_write);
				close_handle(m_state->stdin_read);
				close_handle(m_state->stdin_write);
				close_handle(m_state->job);
				return false;
			}
			STARTUPINFOW startup_info{ .cb = sizeof(STARTUPINFOW),
				                       .dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW,
				                       .wShowWindow = SW_HIDE,
				                       .hStdInput = m_state->stdin_read,
				                       .hStdOutput = m_state->stdout_write,
				                       .hStdError = m_state->stderr_write };
			std::wstring command_line{ quote_argument(m_path.wstring()) };
			for (const std::string& argument : m_arguments)
			{
				command_line += L" ";
				command_line += quote_argument(string_manip::wstr(argument));
			}
			std::vector<wchar_t> command_line_buffer(command_line.begin(), command_line.end());
			command_line_buffer.push_back(L'\0');
			const std::wstring working_directory_str{ m_working_directory.empty() ? std::wstring{} : m_working_directory.wstring() };
			if (CreateProcessW(nullptr, command_line_buffer.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, nullptr,
			                   working_directory_str.empty() ? nullptr : working_directory_str.c_str(), &startup_info, &m_state->process_information) == FALSE)
			{
				close_handle(m_state->stdout_read);
				close_handle(m_state->stdout_write);
				close_handle(m_state->stderr_read);
				close_handle(m_state->stderr_write);
				close_handle(m_state->stdin_read);
				close_handle(m_state->stdin_write);
				close_handle(m_state->job);
				return false;
			}
			close_handle(m_state->stdout_write);
			close_handle(m_state->stderr_write);
			close_handle(m_state->stdin_read);
			if (AssignProcessToJobObject(m_state->job, m_state->process_information.hProcess) == FALSE)
			{
				TerminateProcess(m_state->process_information.hProcess, 1);
				return false;
			}
			m_status = process_status::running;
			m_exit_code = -1;
			m_standard_output.clear();
			m_standard_error.clear();
			m_watcher = std::thread(&process::watch, this);
		}
		catch (...)
		{
			if (m_state->process_information.hProcess != nullptr)
			{
				TerminateProcess(m_state->process_information.hProcess, 1);
			}
			close_handle(m_state->stdout_read);
			close_handle(m_state->stdout_write);
			close_handle(m_state->stderr_read);
			close_handle(m_state->stderr_write);
			close_handle(m_state->stdin_read);
			close_handle(m_state->stdin_write);
			close_handle(m_state->process_information.hProcess);
			close_handle(m_state->process_information.hThread);
			close_handle(m_state->job);
			m_status = process_status::created;
			m_exit_code = -1;
			return false;
		}
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
		close_handle(m_state->stdin_write);
		if (m_watcher.joinable())
		{
			m_watcher.join();
		}
		lock.lock();
		return m_exit_code;
	}

	void process::watch()
	{
		try
		{
			DWORD process_exit_code{ STILL_ACTIVE };
			while (true)
			{
				DWORD wait_result{ WaitForSingleObject(m_state->process_information.hProcess, 50) };
				std::unique_lock lock{ m_mutex };
				std::string new_output{ append_pipe_output(m_standard_output, m_state->stdout_read) };
				std::string new_error{ append_pipe_output(m_standard_error, m_state->stderr_read) };
				lock.unlock();
				if (!new_output.empty())
				{
					m_output_received_event.invoke(*this, { new_output });
				}
				if (!new_error.empty())
				{
					m_error_received_event.invoke(*this, { new_error });
				}
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
			std::unique_lock lock{ m_mutex };
			std::string new_output{ append_pipe_output(m_standard_output, m_state->stdout_read) };
			std::string new_error{ append_pipe_output(m_standard_error, m_state->stderr_read) };
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
			if (process_exit_code == STILL_ACTIVE && GetExitCodeProcess(m_state->process_information.hProcess, &process_exit_code) == FALSE)
			{
				process_exit_code = static_cast<DWORD>(-1);
			}
			m_exit_code = static_cast<int>(process_exit_code);
			if (m_status != process_status::killed)
			{
				m_status = process_status::completed;
			}
			lock.unlock();
			m_exited_event.invoke(*this, { m_exit_code });
		}
		catch (...)
		{
		}
	}
}