#pragma once

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
		void add(const std::string& argument);
		bool contains(std::string_view argument) const;
		const std::vector<std::string>& get_all() const;
		size_t get_count() const;
		std::optional<std::string> get_next(std::string_view argument) const;

	private:
		std::vector<std::string> m_arguments;
	};
}