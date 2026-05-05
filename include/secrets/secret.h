#pragma once

#include <string>

namespace desktop::secrets
{
	class secret
	{
	public:
		secret(const std::string& name, const std::string& value);
		~secret() = default;
		secret(const secret& other) = default;
		secret(secret&& other) noexcept = default;
		const std::string& get_name() const;
		const std::string& get_value() const;
		bool empty() const;
		secret& operator=(const secret& other) = default;
		secret& operator=(secret&& other) noexcept = default;
		operator bool() const;

	private:
		std::string m_name;
		std::string m_value;
	};
}