#include "app/ipc_service.h"
#include <windows.h>
#include <array>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include "helpers/string_manip.h"

using namespace desktop::helpers;

static constexpr DWORD s_buffer_size{ 4096 };

namespace desktop::app
{
	class ipc_service::impl
	{
	public:
		impl(ipc_service& owner)
		    : m_owner{ owner }
		{
			std::wstring pipe_name{ string_manip::wstr("\\\\.\\pipe\\" + m_owner.m_app_info->get_id()) };
			HANDLE pipe{ CreateNamedPipeW(pipe_name.c_str(), PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				                          PIPE_UNLIMITED_INSTANCES, s_buffer_size, s_buffer_size, 0, nullptr) };

			if (pipe != INVALID_HANDLE_VALUE)
			{
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
		}

		~impl()
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

		bool is_host() const
		{
			return m_host;
		}

		bool send_message(const std::string& message)
		{
			std::wstring pipe_name{ string_manip::wstr("\\\\.\\pipe\\" + m_owner.m_app_info->get_id()) };
			HANDLE pipe{ CreateFileW(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr) };
			if (pipe == INVALID_HANDLE_VALUE)
			{
				return false;
			}
			DWORD mode{ PIPE_READMODE_MESSAGE };
			SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr);
			DWORD written{ 0 };
			bool ok{ static_cast<bool>(WriteFile(pipe, message.c_str(), static_cast<DWORD>(message.size()), &written, nullptr)) };
			CloseHandle(pipe);
			return ok;
		}

	private:
		void listen_loop()
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
				ConnectNamedPipe(m_pipe, &overlapped);
				DWORD wait{ WaitForMultipleObjects(2, wait_handles.data(), FALSE, INFINITE) };
				if (wait != WAIT_OBJECT_0)
				{
					break;
				}
				DWORD bytes{ 0 };
				while (true)
				{
					ResetEvent(overlapped.hEvent);
					bool ok{ static_cast<bool>(ReadFile(m_pipe, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes, &overlapped)) };
					if (!ok && GetLastError() == ERROR_IO_PENDING)
					{
						wait = WaitForMultipleObjects(2, wait_handles.data(), FALSE, INFINITE);
						if (wait != WAIT_OBJECT_0)
						{
							CloseHandle(overlapped.hEvent);
							return;
						}
						ok = static_cast<bool>(GetOverlappedResult(m_pipe, &overlapped, &bytes, FALSE));
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

		ipc_service& m_owner;
		bool m_host{ false };
		HANDLE m_pipe{ nullptr };
		HANDLE m_terminate_event{ nullptr };
		std::thread m_thread;
	};

	ipc_service::ipc_service(std::shared_ptr<app_info> app_info)
	    : m_app_info{ std::move(app_info) },
	      m_impl{ std::make_unique<impl>(*this) }
	{
	}

	const events::event<ipc_service, events::param_event_args<std::string>>& ipc_service::get_message_received_event() const
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
