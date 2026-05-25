#pragma once

#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace desktop::app
{
	class arguments_service
	{
	public:
		arguments_service(std::span<char*> argv);
		~arguments_service() = default;
		arguments_service(const arguments_service&) = delete;
		arguments_service(arguments_service&&) = delete;
		void add(const std::string& argument);
		char** argv() const;
		bool contains(std::string_view argument) const;
		std::vector<std::string> get_all() const;
		size_t get_count() const;
		std::optional<std::string> get_next(std::string_view argument) const;
		arguments_service& operator=(const arguments_service&) = delete;
		arguments_service& operator=(arguments_service&&) = delete;

	private:
		mutable std::mutex m_mutex;
		mutable std::vector<char*> m_argv;
		std::vector<std::string> m_arguments;
	};
}