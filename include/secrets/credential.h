#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace desktop::secrets
{
	class credential
	{
	public:
		credential() = default;
		credential(std::string name, std::string username, std::string password, std::string url);
		~credential() = default;
		credential(const credential& other) = default;
		credential(credential&& other) noexcept = default;
		const std::string& get_name() const;
		void set_name(const std::string& name);
		const std::string& get_username() const;
		void set_username(const std::string& username);
		const std::string& get_password() const;
		void set_password(const std::string& password);
		const std::string& get_url() const;
		void set_url(const std::string& url);
		std::strong_ordering operator<=>(const credential& other) const;
		bool operator==(const credential& other) const;
		bool operator!=(const credential& other) const;
		credential& operator=(const credential& other) = default;
		credential& operator=(credential&& other) noexcept = default;

		friend void to_json(nlohmann::json& j, const credential& v)
		{
			j = { { "name", v.get_name() }, { "username", v.get_username() }, { "password", v.get_password() }, { "url", v.get_url() } };
		}

		friend void from_json(const nlohmann::json& j, credential& c)
		{
			c = credential{ j.at("name").get<std::string>(), j.at("username").get<std::string>(), j.at("password").get<std::string>(),
				            j.at("url").get<std::string>() };
		}

	private:
		std::string m_name;
		std::string m_username;
		std::string m_password;
		std::string m_url;
	};
}