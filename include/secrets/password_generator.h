#pragma once

#include <random>
#include <string>
#include "secrets/password_content.h"

namespace desktop::secrets
{
	class password_generator
	{
	public:
		password_generator(password_content content_flags = password_content::numeric | password_content::uppercase | password_content::lowercase | password_content::special | password_content::space) noexcept;
		~password_generator() = default;
		password_generator(const password_generator& other) noexcept;
		password_generator(password_generator&& other) noexcept;
		password_content content_flags() const noexcept;
		void set_content_flags(password_content content_flags) noexcept;
		std::string next(std::size_t length = 16) noexcept;
		password_generator& operator=(const password_generator& other) noexcept;
		password_generator& operator=(password_generator&& other) noexcept;

	private:
		password_content m_content_flags;
		std::random_device m_random_device;
		std::mt19937 m_random_engine;
	};
}
