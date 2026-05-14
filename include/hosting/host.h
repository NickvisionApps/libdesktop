#pragma once

#include <exception>
#include <memory>
#include "host_options.h"
#include "services/service_collection.h"

namespace desktop::hosting
{
	class host
	{
	public:
		host(const host_options& options);
		~host() = default;
		host(const host&) = default;
		host(host&&) noexcept = default;
		std::shared_ptr<services::service_collection>& get_services();
		std::exception_ptr run();
		void use_github_updates();
		host& operator=(const host&) = default;
		host& operator=(host&&) noexcept = default;

	private:
		std::shared_ptr<services::service_collection> m_services;
	};
}