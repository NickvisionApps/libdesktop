#pragma once

#include <memory>
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
		class impl;
		friend class impl;
		std::unique_ptr<impl> m_impl;
		std::string m_name;
	};
}