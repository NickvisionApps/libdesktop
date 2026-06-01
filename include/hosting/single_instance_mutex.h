#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace desktop::hosting
{
	class single_instance_mutex
	{
	public:
		single_instance_mutex(std::string name);
		~single_instance_mutex();
		single_instance_mutex(const single_instance_mutex&) = delete;
		single_instance_mutex(single_instance_mutex&&) = delete;
		bool is_locked() const;
		bool lock();
		void unlock();
		single_instance_mutex& operator=(const single_instance_mutex&) = delete;
		single_instance_mutex& operator=(single_instance_mutex&&) = delete;

	private:
		class state;
		friend class state;
		static std::mutex s_registry_mutex;
		static std::set<std::string> s_locked_names;
		std::unique_ptr<state> m_state;
		std::string m_name;
	};
}