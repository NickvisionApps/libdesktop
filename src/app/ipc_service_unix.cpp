#include "app/ipc_service.h"
#include <array>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <utility>

static constexpr int s_backlog{ 8 };
static constexpr std::size_t s_buffer_size{ 4096 };

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
		int m_server_fd{ -1 };
		std::array<int, 2> m_pipe_fds{ -1, -1 };
		std::string m_socket_path;
		std::thread m_thread;
	};

	ipc_service::impl::impl(ipc_service& owner)
	    : m_owner{ owner }
	{
		m_socket_path = "/tmp/" + m_owner.m_app_info->get_id() + ".sock";
		int fd{ socket(AF_UNIX, SOCK_STREAM, 0) };
		if (fd < 0)
		{
			throw std::runtime_error("Unable to create socket");
		}
		sockaddr_un addr{};
		addr.sun_family = AF_UNIX;
		std::strncpy(addr.sun_path, m_socket_path.c_str(), sizeof(addr.sun_path) - 1);
		if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0)
		{
			m_host = true;
			m_server_fd = fd;
			if (listen(m_server_fd, s_backlog) < 0)
			{
				::close(m_server_fd);
				::unlink(m_socket_path.c_str());
				throw std::runtime_error("Unable to listen on socket");
			}
			if (pipe(m_pipe_fds.data()) < 0)
			{
				::close(m_server_fd);
				::unlink(m_socket_path.c_str());
				throw std::runtime_error("Unable to create pipe");
			}
			m_thread = std::thread(&impl::listen_loop, this);
		}
		else
		{
			::close(fd);
		}
	}

	ipc_service::impl::~impl()
	{
		if (m_host)
		{
			char byte{ 0 };
			write(m_pipe_fds[1], &byte, 1);
			if (m_thread.joinable())
			{
				m_thread.join();
			}
			::close(m_pipe_fds[0]);
			::close(m_pipe_fds[1]);
			::close(m_server_fd);
			::unlink(m_socket_path.c_str());
		}
	}

	bool ipc_service::impl::is_host() const
	{
		return m_host;
	}

	bool ipc_service::impl::send_message(const std::string& message)
	{
		int fd{ socket(AF_UNIX, SOCK_STREAM, 0) };
		if (fd < 0)
		{
			return false;
		}
		sockaddr_un addr{};
		addr.sun_family = AF_UNIX;
		std::strncpy(addr.sun_path, m_socket_path.c_str(), sizeof(addr.sun_path) - 1);
		if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			::close(fd);
			return false;
		}
		ssize_t written{ write(fd, message.data(), message.size()) };
		::close(fd);
		return std::cmp_equal(written, message.size());
	}

	void ipc_service::impl::listen_loop()
	{
		std::string buffer(s_buffer_size, '\0');
		while (true)
		{
			fd_set read_fds{};
			FD_ZERO(&read_fds);
			FD_SET(m_server_fd, &read_fds);
			FD_SET(m_pipe_fds[0], &read_fds);
			int max_fd{ std::max(m_server_fd, m_pipe_fds[0]) };
			if (select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr) < 0)
			{
				break;
			}
			if (FD_ISSET(m_pipe_fds[0], &read_fds))
			{
				break;
			}
			if (!FD_ISSET(m_server_fd, &read_fds))
			{
				continue;
			}
			int client_fd{ accept(m_server_fd, nullptr, nullptr) };
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
				m_owner.m_message_received_event.invoke(m_owner, { std::move(message) });
			}
		}
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