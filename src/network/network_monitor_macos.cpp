#include "network/network_monitor.h"
#include <SystemConfiguration/SystemConfiguration.h>
#include <netinet/in.h>
#include <stdexcept>

using namespace desktop::events;

namespace desktop::network
{
	class network_monitor::impl
	{
	public:
		impl(network_monitor& owner)
		    : m_owner{ owner }
		{
			sockaddr_in addr{};
			addr.sin_len = sizeof(addr);
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(0x08080808);
			m_reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, reinterpret_cast<sockaddr*>(&addr));
			if (!m_reachability)
			{
				throw std::runtime_error("Unable to create network reachability target.");
			}
			SCNetworkReachabilityContext context{ .version = 0, .info = this, .retain = nullptr, .release = nullptr, .copyDescription = nullptr };
			if (!SCNetworkReachabilitySetCallback(m_reachability, [](SCNetworkReachabilityRef, SCNetworkReachabilityFlags, void* data)
			{
				static_cast<impl*>(data)->check_connection_state(true);
			}, &context))
			{
				CFRelease(m_reachability);
				throw std::runtime_error("Unable to set network reachability callback.");
			}
			if (!SCNetworkReachabilityScheduleWithRunLoop(m_reachability, CFRunLoopGetMain(), kCFRunLoopDefaultMode))
			{
				SCNetworkReachabilitySetCallback(m_reachability, nullptr, nullptr);
				CFRelease(m_reachability);
				throw std::runtime_error("Unable to schedule network reachability with run loop.");
			}
			check_connection_state(false);
		}

		~impl() noexcept
		{
			if (m_reachability)
			{
				SCNetworkReachabilityUnscheduleFromRunLoop(m_reachability, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
				SCNetworkReachabilitySetCallback(m_reachability, nullptr, nullptr);
				CFRelease(m_reachability);
			}
		}

		impl(const impl&) = delete;

		impl(impl&&) noexcept = delete;

		network_state get_current_state() const
		{
			std::scoped_lock lock{ m_mutex };
			return m_current_state;
		}

		impl& operator=(const impl&) = delete;

		impl& operator=(impl&&) noexcept = delete;

	private:
		void check_connection_state(bool event) noexcept
		{
			network_state new_state{ network_state::disconnected };
			SCNetworkReachabilityFlags flags{};
			if (SCNetworkReachabilityGetFlags(m_reachability, &flags))
			{
				bool reachable{ static_cast<bool>(flags & kSCNetworkReachabilityFlagsReachable) };
				bool connection_required{ static_cast<bool>(flags & kSCNetworkReachabilityFlagsConnectionRequired) };
				if (reachable && !connection_required)
				{
					new_state = network_state::connected_global;
				}
				else if (reachable)
				{
					new_state = network_state::connected_local;
				}
			}
			std::unique_lock<std::mutex> lock{ m_mutex };
			if (m_current_state != new_state)
			{
				m_current_state = new_state;
				lock.unlock();
				if (event)
				{
					m_owner.m_state_changed_event.invoke(m_owner, { new_state });
				}
			}
		}

		mutable std::mutex m_mutex;
		network_monitor& m_owner;
		network_state m_current_state{ network_state::disconnected };
		SCNetworkReachabilityRef m_reachability{ nullptr };
	};

	network_monitor::network_monitor()
	    : m_impl{ std::make_unique<impl>(*this) }
	{
	}

	network_monitor::~network_monitor() = default;

	const event<network_monitor, param_event_args<network_state>>& network_monitor::get_state_changed_event() const
	{
		return m_state_changed_event;
	}

	network_state network_monitor::get_current_state() const
	{
		return m_impl->get_current_state();
	}
}