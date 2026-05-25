#pragma once

#include <chrono>
#include <cstddef>
#include <curl/curl.h>
#include <format>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace desktop::network
{
	class http_response
	{
	public:
		http_response();
		~http_response() = default;
		http_response(const http_response&) = default;
		http_response(http_response&&) = default;
		bool is_success() const;
		long get_response_code() const;
		void set_response_code(long code);
		const std::chrono::seconds& get_elapsed() const;
		void set_elapsed(const std::chrono::seconds& elapsed);
		std::string get_content_as_string() const;
		nlohmann::json get_content_as_json() const;
		template <typename T>
		    requires std::is_default_constructible_v<T>
		std::optional<T> get_content_as_object() const
		{
			nlohmann::json json = get_content_as_json();
			if (!json.is_null() && !json.empty())
			{
				try
				{
					T obj;
					nlohmann::from_json(json, obj);
					return obj;
				}
				catch (...)
				{
					throw std::runtime_error(std::format("Unable to convert json to object ({})", typeid(T).name()));
				}
			}
			return std::nullopt;
		}
		void write_content(const char* ptr, size_t size);
		void set_info_from_easy(CURL* easy);
		http_response& operator=(const http_response&) = default;
		http_response& operator=(http_response&&) = default;

	private:
		long m_response_code{ 0 };
		std::chrono::seconds m_elapsed;
		std::vector<char> m_content;
	};
}