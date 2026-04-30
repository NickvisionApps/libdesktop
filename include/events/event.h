#pragma once

#include <cstddef>
#include <functional>
#include <concepts>
#include <vector>
#include "event_args.h"

namespace desktop::events
{
	using event_id = std::size_t;

    template<typename T, std::derived_from<event_args> U>
    class event
    {
    public:
        event()
        {

        }

        event_id add_handler(std::function<void(const T&, const U&)> handler) const
        {
            event_id id{ m_handlers.size() };
            m_handlers.push_back(std::move(handler));
            return id;
		}

        void invoke(const T& sender, const U& args)
        {
            for (const auto& handler : m_handlers)
            {
                handler(sender, args);
            }
		}

        void remove_handler(event_id id) const
        {
			if (id < m_handlers.size())
			{
				m_handlers.erase(m_handlers.begin() + id);
			}
		}

        event& operator+=(std::function<void(const T&, const U&)> handler) const
        {
            add_handler(handler);
            return *this;
		}

        event& operator-=(event_id id) const
        {
            remove_handler(id);
            return *this;
        }

    private:
		mutable std::vector<std::function<void(const T&, const U&)>> m_handlers;
    };
}