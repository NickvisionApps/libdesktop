#include "app/ipc_service.h"
#include <windows.h>
#include <array>
#include <stdexcept>
#include <string>

static constexpr DWORD s_buffer_size{ 4096 };

using namespace desktop::events;

namespace desktop::app
{
	class ipc_service::state
	{
	public:
		HANDLE pipe{ nullptr };
		HANDLE terminate_event{ nullptr };
	};

	ipc_service::ipc_service(std::shared_ptr<app_info> app_info)
	    : m_state{ std::make_unique<state>() },
	      m_app_info{ std::move(app_info) }
	{
		std::string pipe_name{ "\\\\.\\pipe\\" + m_app_info->get_id() };
		HANDLE existing{ CreateFileA(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr) };
		if (existing != INVALID_HANDLE_VALUE)
		{
			CloseHandle(existing);
			return;
		}
		if (GetLastError() == ERROR_PIPE_BUSY)
		{
			return;
		}
		HANDLE pipe{ CreateNamedPipeA(pipe_name.c_str(), PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1,
			                          s_buffer_size, s_buffer_size, 0, nullptr) };
		if (pipe == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error("Unable to create pipe");
		}
		m_host = true;
		m_state->pipe = pipe;
		m_state->terminate_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!m_state->terminate_event)
		{
			CloseHandle(m_state->pipe);
			throw std::runtime_error("Unable to create terminate event");
		}
		m_listener = std::thread(&ipc_service::watch, this);
	}

	ipc_service::~ipc_service()
	{
		if (m_host && m_state->terminate_event)
		{
			SetEvent(m_state->terminate_event);
			if (m_listener.joinable())
			{
				m_listener.join();
			}
			CloseHandle(m_state->terminate_event);
		}
		if (m_state->pipe && m_state->pipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_state->pipe);
		}
	}

	const event<ipc_service, param_event_args<std::string>>& ipc_service::get_message_received_event() const
	{
		return m_message_received_event;
	}

	bool ipc_service::is_host() const
	{
		return m_host;
	}

	bool ipc_service::send_message(const std::string& message)
	{
		std::string pipe_name{ "\\\\.\\pipe\\" + m_app_info->get_id() };
		if (!WaitNamedPipeA(pipe_name.c_str(), NMPWAIT_WAIT_FOREVER))
		{
			return false;
		}
		HANDLE pipe{ CreateFileA(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr) };
		if (pipe == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		DWORD written{ 0 };
		BOOL ok{ WriteFile(pipe, message.c_str(), static_cast<DWORD>(message.size()), &written, nullptr) };
		CloseHandle(pipe);
		return ok != 0;
	}

	void ipc_service::watch()
	{
		HANDLE connect_event{ CreateEventW(nullptr, TRUE, FALSE, nullptr) };
		if (!connect_event)
		{
			return;
		}
		std::array<HANDLE, 2> connect_wait{ connect_event, m_state->terminate_event };
		std::string buffer(s_buffer_size, '\0');
		while (true)
		{
			OVERLAPPED connect_ov{};
			connect_ov.hEvent = connect_event;
			ResetEvent(connect_event);
			BOOL connected{ ConnectNamedPipe(m_state->pipe, &connect_ov) };
			if (!connected)
			{
				DWORD err{ GetLastError() };
				if (err == ERROR_IO_PENDING)
				{
					DWORD wait{ WaitForMultipleObjects(static_cast<DWORD>(connect_wait.size()), connect_wait.data(), FALSE, INFINITE) };
					if (wait != WAIT_OBJECT_0)
					{
						CancelIoEx(m_state->pipe, &connect_ov);
						break;
					}
					DWORD dummy{};
					if (!GetOverlappedResult(m_state->pipe, &connect_ov, &dummy, FALSE))
					{
						DisconnectNamedPipe(m_state->pipe);
						continue;
					}
				}
				else if (err != ERROR_PIPE_CONNECTED)
				{
					DisconnectNamedPipe(m_state->pipe);
					continue;
				}
			}
			while (true)
			{
				OVERLAPPED read_ov{};
				read_ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
				if (!read_ov.hEvent)
				{
					break;
				}
				std::array<HANDLE, 2> read_wait{ read_ov.hEvent, m_state->terminate_event };
				DWORD bytes{ 0 };
				BOOL ok{ ReadFile(m_state->pipe, buffer.data(), static_cast<DWORD>(buffer.size()), nullptr, &read_ov) };
				if (!ok)
				{
					DWORD err{ GetLastError() };
					if (err == ERROR_IO_PENDING)
					{
						DWORD wait{ WaitForMultipleObjects(static_cast<DWORD>(read_wait.size()), read_wait.data(), FALSE, INFINITE) };
						if (wait != WAIT_OBJECT_0)
						{
							CancelIoEx(m_state->pipe, &read_ov);
							CloseHandle(read_ov.hEvent);
							CloseHandle(connect_event);
							return;
						}
						ok = GetOverlappedResult(m_state->pipe, &read_ov, &bytes, FALSE);
					}
					else
					{
						CloseHandle(read_ov.hEvent);
						break;
					}
				}
				else
				{
					GetOverlappedResult(m_state->pipe, &read_ov, &bytes, FALSE);
				}
				CloseHandle(read_ov.hEvent);
				if (!ok || bytes == 0)
				{
					break;
				}
				m_message_received_event.invoke(*this, { std::string{ buffer.data(), bytes } });
			}
			DisconnectNamedPipe(m_state->pipe);
		}
		CloseHandle(connect_event);
	}
}