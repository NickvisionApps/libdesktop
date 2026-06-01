#include "app/ipc_service.h"
#include <array>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>

static constexpr int s_backlog{ 8 };
static constexpr std::size_t s_buffer_size{ 4096 };

using namespace desktop::events;

namespace desktop::app
{
	class ipc_service::state
	{
	public:
		int server_fd{ -1 };
		std::array<int, 2> pipe_fds{ -1, -1 };
		std::string socket_path;
	};

	ipc_service::ipc_service(std::shared_ptr<app_info> app_info)
	    : m_state{ std::make_unique<state>() },
	      m_app_info{ std::move(app_info) }
	{
		m_state->socket_path = "/tmp/" + m_app_info->get_id() + ".sock";
		int fd{ socket(AF_UNIX, SOCK_STREAM, 0) };
		if (fd < 0)
		{
			throw std::runtime_error("Unable to create socket");
		}
		sockaddr_un addr{};
		addr.sun_family = AF_UNIX;
		std::strncpy(addr.sun_path, m_state->socket_path.c_str(), sizeof(addr.sun_path) - 1);
		if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0)
		{
			m_host = true;
			m_state->server_fd = fd;
			if (listen(m_state->server_fd, s_backlog) < 0)
			{
				::close(m_state->server_fd);
				::unlink(m_state->socket_path.c_str());
				throw std::runtime_error("Unable to listen on socket");
			}
			if (pipe(m_state->pipe_fds.data()) < 0)
			{
				::close(m_state->server_fd);
				::unlink(m_state->socket_path.c_str());
				throw std::runtime_error("Unable to create pipe");
			}
			m_listener = std::thread(&ipc_service::watch, this);
		}
		else
		{
			::close(fd);
		}
	}

	ipc_service::~ipc_service()
	{
		if (!m_host)
		{
			return;
		}
		char byte{ 0 };
		write(m_state->pipe_fds[1], &byte, 1);
		if (m_listener.joinable())
		{
			m_listener.join();
		}
		::close(m_state->pipe_fds[0]);
		::close(m_state->pipe_fds[1]);
		::close(m_state->server_fd);
		::unlink(m_state->socket_path.c_str());
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
		int fd{ socket(AF_UNIX, SOCK_STREAM, 0) };
		if (fd < 0)
		{
			return false;
		}
		sockaddr_un addr{};
		addr.sun_family = AF_UNIX;
		std::strncpy(addr.sun_path, m_state->socket_path.c_str(), sizeof(addr.sun_path) - 1);
		if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			::close(fd);
			return false;
		}
		ssize_t written{ write(fd, message.data(), message.size()) };
		::close(fd);
		return std::cmp_equal(written, message.size());
	}

	void ipc_service::watch()
	{
		std::string buffer(s_buffer_size, '\0');
		while (true)
		{
			fd_set read_fds{};
			FD_ZERO(&read_fds);
			FD_SET(m_state->server_fd, &read_fds);
			FD_SET(m_state->pipe_fds[0], &read_fds);
			int max_fd{ std::max(m_state->server_fd, m_state->pipe_fds[0]) };
			if (select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr) < 0)
			{
				break;
			}
			if (FD_ISSET(m_state->pipe_fds[0], &read_fds))
			{
				break;
			}
			if (!FD_ISSET(m_state->server_fd, &read_fds))
			{
				continue;
			}
			int client_fd{ accept(m_state->server_fd, nullptr, nullptr) };
			if (client_fd < 0)
			{
				continue;
			}
			std::string message{};
			while (true)
			{
				ssize_t bytes{ read(client_fd, buffer.data(), buffer.size()) };
				if (bytes <= 0)
				{
					break;
				}
				message.append(buffer.data(), static_cast<std::size_t>(bytes));
			}
			::close(client_fd);
			if (!message.empty())
			{
				m_message_received_event.invoke(*this, { std::move(message) });
			}
		}
	}
}