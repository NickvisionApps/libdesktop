#include "app/ipc_service.h"
#include <windows.h>
#include <array>
#include <stdexcept>
#include <string>
#include <thread>

static constexpr DWORD s_buffer_size{ 4096 };

using namespace desktop::events;

namespace desktop::app
{
	class ipc_service::impl
	{
	public:
		impl(ipc_service& owner);
		~impl();
		impl(const impl&) = delete;
		impl(impl&&) = delete;
		impl& operator=(const impl&) = delete;
		impl& operator=(impl&&) = delete;
		bool is_host() const;
		bool send_message(const std::string& message);

	private:
		void listen_loop();
		ipc_service& m_owner;
		bool m_host{ false };
		HANDLE m_pipe{ nullptr };
		HANDLE m_terminate_event{ nullptr };
		std::thread m_thread;
	};

	ipc_service::impl::impl(ipc_service& owner)
	    : m_owner{ owner }
	{
		std::string pipe_name{ "\\\\.\\pipe\\" + m_owner.m_app_info->get_id() };
		HANDLE existing = CreateFileA(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
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
		m_pipe = pipe;
		m_terminate_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!m_terminate_event)
		{
			CloseHandle(m_pipe);
			throw std::runtime_error("Unable to create terminate event");
		}
		m_thread = std::thread(&impl::listen_loop, this);
	}

	ipc_service::impl::~impl()
	{
		if (m_host && m_terminate_event)
		{
			SetEvent(m_terminate_event);
			if (m_thread.joinable())
			{
				m_thread.join();
			}
			CloseHandle(m_terminate_event);
		}
		if (m_pipe && m_pipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_pipe);
		}
	}

	bool ipc_service::impl::is_host() const
	{
		return m_host;
	}

	bool ipc_service::impl::send_message(const std::string& message)
	{
		std::string pipe_name{ "\\\\.\\pipe\\" + m_owner.m_app_info->get_id() };
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

	void ipc_service::impl::listen_loop()
	{
		OVERLAPPED overlapped{};
		overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!overlapped.hEvent)
		{
			return;
		}
		std::array<HANDLE, 2> wait_handles{ overlapped.hEvent, m_terminate_event };
		std::string buffer(s_buffer_size, '\0');
		while (true)
		{
			ResetEvent(overlapped.hEvent);
			BOOL connected{ ConnectNamedPipe(m_pipe, &overlapped) };
			if (!connected)
			{
				DWORD error{ GetLastError() };
				if (error == ERROR_IO_PENDING)
				{
					DWORD wait{ WaitForMultipleObjects(static_cast<DWORD>(wait_handles.size()), wait_handles.data(), FALSE, INFINITE) };
					if (wait != WAIT_OBJECT_0)
					{
						break;
					}
				}
				else if (error != ERROR_PIPE_CONNECTED)
				{
					break;
				}
			}
			while (true)
			{
				DWORD bytes{ 0 };
				ResetEvent(overlapped.hEvent);
				BOOL ok{ ReadFile(m_pipe, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes, &overlapped) };
				if (!ok && GetLastError() == ERROR_IO_PENDING)
				{
					DWORD wait{ WaitForMultipleObjects(static_cast<DWORD>(wait_handles.size()), wait_handles.data(), FALSE, INFINITE) };
					if (wait != WAIT_OBJECT_0)
					{
						CloseHandle(overlapped.hEvent);
						return;
					}
					ok = GetOverlappedResult(m_pipe, &overlapped, &bytes, FALSE);
				}
				if (!ok || bytes == 0)
				{
					break;
				}
				m_owner.m_message_received_event.invoke(m_owner, { std::string{ buffer.data(), bytes } });
			}
			DisconnectNamedPipe(m_pipe);
		}
		CloseHandle(overlapped.hEvent);
	}

	ipc_service::ipc_service(std::shared_ptr<app_info> app_info)
	    : m_app_info{ std::move(app_info) },
	      m_impl{ std::make_unique<impl>(*this) }
	{
	}

	ipc_service::~ipc_service() = default;

	const event<ipc_service, param_event_args<std::string>>& ipc_service::get_message_received_event() const
	{
		return m_message_received_event;
	}

	bool ipc_service::is_host() const
	{
		return m_impl->is_host();
	}

	bool ipc_service::send_message(const std::string& message)
	{
		return m_impl->send_message(message);
	}
}