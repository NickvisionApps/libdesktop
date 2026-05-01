#pragma once

#include <concepts>
#include <functional>
#include <mutex>
#include <unordered_map>
#include "event_args.h"

namespace desktop::events
{
    using event_id = unsigned int;

    template<typename T, std::derived_from<event_args> U>
    class event
    {
    public:
        event() = default;
        ~event() = default;
        event(const event&) = delete;
        event(event&&) = delete;
        event& operator=(const event&) = delete;
        event& operator=(event&&) = delete;

        event_id add_handler(std::function<void(const T&, const U&)> handler) const
        {
			std::lock_guard lock{ m_mutex };
            event_id id{ m_next++ };
            m_handlers.insert({ id, std::move(handler) });
            return id;
		}

        void invoke(const T& sender, const U& args)
        {
            std::lock_guard lock{ m_mutex };
            for (const std::pair<const event_id, std::function<void(const T&, const U&)>>& pair : m_handlers)
            {
                if (pair.second)
                {
                    pair.second(sender, args);
                }
                else
                {
					m_handlers.erase(pair.first);
                }
            }
		}

        void remove_handler(event_id id) const
        {
            std::lock_guard lock{ m_mutex };
			m_handlers.erase(id);
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
		mutable std::mutex m_mutex;
        mutable event_id m_next;
		mutable std::unordered_map<event_id, std::function<void(const T&, const U&)>> m_handlers;
    };
}