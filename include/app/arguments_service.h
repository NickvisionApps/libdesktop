#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "services/service.h"

namespace desktop::app
{
	class arguments_service : public services::service
	{
	public:
		arguments_service(int argc, char* argv[]);
		~arguments_service() override = default;
		arguments_service(const arguments_service&) = delete;
		arguments_service(arguments_service&&) = delete;
		void add(const std::string& argument);
		bool contains(std::string_view argument) const;
		const std::vector<std::string>& get_all() const;
		size_t get_count() const;
		std::optional<std::string> get_next(std::string_view argument) const;
		arguments_service& operator=(const arguments_service&) = delete;
		arguments_service& operator=(arguments_service&&) = delete;

	private:
		mutable std::mutex m_mutex;
		std::vector<std::string> m_arguments;
	};
}