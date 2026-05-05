#include <array>
#include <cmath>
#include "secrets/password_generator.h"

namespace desktop::secrets
{
	static const std::array<char, 10> s_numeric{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	static const std::array<char, 26> s_uppercase{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		                                           'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
	static const std::array<char, 26> s_lowercase{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		                                           'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
	static const std::array<char, 32> s_special{ '!', '"', '#', '$', '%', '&', '\'', '(',  ')', '*', '+', ',', '-', '.', '/', ':',
		                                         ';', '<', '=', '>', '?', '@', '[',  '\\', ']', '^', '_', '`', '{', '|', '}', '~' };

	password_generator::password_generator(password_content content_flags) noexcept
	    : m_content_flags{ content_flags },
	      m_random_engine{ m_random_device() }
	{
	}

	password_generator::password_generator(const password_generator& other)
	    : m_content_flags{ other.m_content_flags },
	      m_random_engine{ m_random_device() }
	{
	}

	password_generator::password_generator(password_generator&& other) noexcept
	    : m_content_flags{ other.m_content_flags },
	      m_random_engine{ other.m_random_engine }
	{
	}

	password_content password_generator::get_content_flags() const noexcept
	{
		return m_content_flags;
	}

	void password_generator::set_content_flags(password_content content_flags) noexcept
	{
		m_content_flags = content_flags;
	}

	std::string password_generator::generate(std::size_t length) noexcept
	{
		static std::uniform_int_distribution<int> type_dist{ 0, 4 };
		static std::uniform_int_distribution<std::size_t> numeric_dist{ 0, s_numeric.size() - 1 };
		static std::uniform_int_distribution<std::size_t> upper_dist{ 0, s_uppercase.size() - 1 };
		static std::uniform_int_distribution<std::size_t> lower_dist{ 0, s_lowercase.size() - 1 };
		static std::uniform_int_distribution<std::size_t> special_dist{ 0, s_special.size() - 1 };
		std::string password;
		password.reserve(length);
		for (std::size_t i{ 0 }; i < length; i++)
		{
			while (true)
			{
				password_content random_type{ static_cast<int>(std::pow(2, type_dist(m_random_engine))) };
				if ((m_content_flags & random_type) == password_content::numeric)
				{
					password += s_numeric[numeric_dist(m_random_engine)];
				}
				else if ((m_content_flags & random_type) == password_content::uppercase)
				{
					password += s_uppercase[upper_dist(m_random_engine)];
				}
				else if ((m_content_flags & random_type) == password_content::lowercase)
				{
					password += s_lowercase[lower_dist(m_random_engine)];
				}
				else if ((m_content_flags & random_type) == password_content::special)
				{
					password += s_special[special_dist(m_random_engine)];
				}
				else if ((m_content_flags & random_type) == password_content::space)
				{
					password += ' ';
				}
				else
				{
					continue;
				}
				break;
			}
		}
		return password;
	}

	password_generator& password_generator::operator=(const password_generator& other)
	{
		if (this != &other)
		{
			m_content_flags = other.m_content_flags;
			m_random_engine = std::mt19937{ m_random_device() };
		}
		return *this;
	}

	password_generator& password_generator::operator=(password_generator&& other) noexcept
	{
		if (this != &other)
		{
			m_content_flags = other.m_content_flags;
			m_random_engine = other.m_random_engine;
		}
		return *this;
	}
}
